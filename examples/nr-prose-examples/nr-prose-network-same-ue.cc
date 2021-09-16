/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


/**
 * \file nr-prose-network-same-ue.cc
 * \ingroup examples
 *
 * \brief Basic scenario with some UEs doing in-network communication, some UEs
 *        doing PoSe unicast communication over SL, and a UE doing both.
 *
 * TODO: This is a toy example. If we keep it, consider adding input parameters
 *       and improving descriptions
 *
 * Channel configuration:
 * This example setup a simulation using the 3GPP channel model from TR 37.885
 * and it uses the default configuration of its implementation.
 *
 * System configuration:
 * The scenario uses one operational band, containing one component carrier,
 * and two bandwidth parts. One bandwidth part is used for in-network
 * communication, i.e., UL and DL between in-network UEs and gNBs, and the
 * other bandwidth part is used for SL communication between UEs using SL.
 *
 * Topology:
 * The scenario is composed of one gNB and four UEs. Two of the UEs (UE0 and
 * UE1) are attached to the gNB and the other two UEs (UE2 and UE3) are
 * out-of-network.
 *
 *        -  gNB              (0.0, 30.0, 10.0)
 *        |
 *   20 m |
 *        -  UE1 UE0          (0.0, 10.0, 1.5) (1.0, 10.0, 1.5)
 *   10 m |       |
 *        -  UE2  |  UE3      (0.0, 0.0, 1.5) (2.0, 0.0, 1.5)
 *            |---|---|
 *             1 m 1 m
 *            |-------|
 *               2 m
 *
 * ProSe Unicast
 * UE1, UE2 and UE3 will start the establishment of the ProSe unicast direct
 * links before the start of the traffic. The configuration is the following:
 *     Link     | Initiating UE  |   Target UE
 * ---------------------------------------------
 * UE1 <-> UE2  |      UE2       |     UE1
 * UE1 <-> UE3  |      UE3       |     UE1
 * UE2 <-> UE3  |      UE2       |     UE3
 *
 * Traffic:
 * There are two CBR traffic flows concerning the in-network UEs (UE0 and UE1),
 * one from a Remote Host in the internet towards each in-network  U (DL) and
 * one from the in-network UEs towards the Remote Host (UL). Additionally, two
 * CBR traffic flows involve the UEs doing ProSe unicast communication on the
 * SL (UE1, UE2, and UE3), one on each direction of each established unicast
 * direct link.
 *
 * Output:
 * The example will print on-screen the end-to-end statistics of each traffic
 * flow, as well as writing them on a file.
 * Also, Sidelink MAC and PHY layer traces as well as application layer traces
 * of the SL UEs are stored in a sqlite3 database using ns-3 stats module.
 *
 * \code{.unparsed}
$ ./waf --run "nr-prose-network-same-ue --Help"
    \endcode
 */


#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ideal-beamforming-algorithm.h"
#include "ns3/antenna-module.h"

#include <sqlite3.h>
//Dependency of these nr-v2x-examples classes for SL statistics
#include "../nr-v2x-examples/ue-mac-pscch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-mac-pssch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pscch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pssch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-to-ue-pkt-txrx-output-stats.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NrProseNetworkSameUe");

/*************************** Methods for tracing SL **************************/
/*Please reffer to nr-prose-unicast-multi-link.cc for function documentation */
void NotifySlPscchScheduling (UeMacPscchTxOutputStats *pscchStats, const SlPscchUeMacStatParameters pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}
void NotifySlPsschScheduling (UeMacPsschTxOutputStats *psschStats, const SlPsschUeMacStatParameters psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}
void NotifySlPscchRx (UePhyPscchRxOutputStats *pscchStats, const SlRxCtrlPacketTraceParams pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}
void NotifySlPsschRx (UePhyPsschRxOutputStats *psschStats, const SlRxDataPacketTraceParams psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}
void
UePacketTraceDb (UeToUePktTxRxOutputStats *stats, Ptr<Node> node, const Address &localAddrs,
                 std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
                 const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  uint32_t nodeId = node->GetId ();
  uint64_t imsi = node->GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint32_t seq = seqTsSizeHeader.GetSeq ();
  uint32_t pktSize = p->GetSize () + seqTsSizeHeader.GetSerializedSize ();

  stats->Save (txRx, localAddrs, nodeId, imsi, pktSize, srcAddrs, dstAddrs, seq);
}
/************************END Methods for tracing SL **************************/


int
main (int argc, char *argv[])
{
  //Common configuration
  uint8_t numBands = 1;
  double centralFrequencyBand = 5.89e9; // band n47 (From SL examples)
  double bandwidthBand = 40e6; //40 MHz
  double centralFrequencyCc0 = 5.89e9;
  double bandwidthCc0 = bandwidthBand;
  std::string pattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|"; // From SL examples
  double bandwidthCc0Bpw0 = bandwidthCc0 / 2;
  double bandwidthCc0Bpw1 = bandwidthCc0 / 2;

  //In-network devices configuration
  uint16_t gNbNum = 1;
  uint16_t inNetUeNumPerGnb = 1;
  double gNbHeight = 10;
  double ueHeight = 1.5;
  uint16_t numerologyCc0Bwp0 = 3; //                   BWP0 will be used for the network
  double gNBtotalTxPower = 8; // dBm
  bool cellScan = false;  // Beamforming method.
  double beamSearchAngleStep = 10.0; // Beamforming parameter.

  //In-network applications configuration
  uint32_t packetSizeDlUl = 1000; //bytes
  uint32_t lambdaDl = 200; // packets per second
  uint32_t lambdaUl = 100; // packets per second
  double inNetTrafficStartTime = 3.0; //seconds


  //Sidelink configuration
  uint16_t numerologyCc0Bwp1 = 2; //(From SL examples)  BWP1 will be used for SL
  uint16_t slUeNum = 2;
  uint16_t slInterUeDistance = 2; //m
  Time startDirLinkTime = Seconds (2.0); //Time to start the Prose Unicast Direct Link establishment procedure

  //Sidelink applications configuration
  uint32_t packetSizeSl = 200; //bytes
  double dataRateSl = 16; //16 kilobits per second
  bool bidirectionalSlTraffic = true;
  Time slTrafficStartTime = Seconds (3.0); //Time to start the traffic in the application layer

  //Configuration of UEs doing in-network and SL
  uint16_t inNetSlUeNum = 1;

  //Simulation configuration
  std::string simTag = "default";
  std::string outputDir = "./";
  double simTime = 10; // seconds

  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.Parse (argc, argv);

  NS_ABORT_IF (numBands < 1);

  //Setup large enough buffer size to avoid overflow
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));


  /**
   * Important!
   * Create and install in-network nodes first to avoid IDs conflicts
   * (e.g., RNTIs assigned by the network and out-of-coverage RNTIs)
   */

  //Create gNBs and in-network UEs, configure positions
  NodeContainer gNbNodes;
  NodeContainer inNetUeNodes;
  MobilityHelper mobility;

  gNbNodes.Create (gNbNum);
  inNetUeNodes.Create (inNetUeNumPerGnb * gNbNum);

  Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  int32_t yValue = 0.0;

  for (uint32_t i = 1; i <= gNbNodes.GetN (); ++i)
    {
      if (i % 2 != 0)
        {
          yValue = static_cast<int> (i) * 30;
        }
      else
        {
          yValue = -yValue;
        }

      gNbPositionAlloc->Add (Vector (0.0, yValue, gNbHeight));

      double xValue = 0.0;
      for (uint32_t j = 1; j <= inNetUeNumPerGnb; ++j)
        {
          if (j % 2 != 0)
            {
              xValue = j;
            }
          else
            {
              xValue = -xValue;
            }

          if (yValue > 0)
            {
              uePositionAlloc->Add (Vector (xValue, 10, ueHeight));
            }
          else
            {
              uePositionAlloc->Add (Vector (xValue, -10, ueHeight));
            }
        }
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (uePositionAlloc);
  mobility.Install (inNetUeNodes);


  //Create UEs nodes doing in-network and SL
  NodeContainer inNetSlUeNodes;
  inNetSlUeNodes.Create (inNetSlUeNum);
  //UE nodes mobility setup
  MobilityHelper mobilityinNetSlUes;
  mobilityinNetSlUes.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> nNetSlUesPositionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < inNetSlUeNum; i++)
    {
      nNetSlUesPositionAlloc->Add (Vector (slInterUeDistance * i, 10.0, 1.5));
    }
  mobilityinNetSlUes.SetPositionAllocator (nNetSlUesPositionAlloc);
  mobilityinNetSlUes.Install (inNetSlUeNodes);


  //Create SL only UEs, configure positions
  NodeContainer slUeNodes;
  slUeNodes.Create (slUeNum);
  //UE nodes mobility setup
  MobilityHelper mobilitySl;
  mobilitySl.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> slUesPositionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < slUeNum; i++)
    {
      slUesPositionAlloc->Add (Vector (slInterUeDistance * i, 0.0, 1.5));
    }
  mobilitySl.SetPositionAllocator (slUesPositionAlloc);
  mobilitySl.Install (slUeNodes);

  //Setup Helpers
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);


  /*************************Spectrum division ****************************/

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;

  OperationBandInfo band;

  /*
   * The configured spectrum division is:
   * |-------------- Band ------------|
   * |---------------CC0--------------|
   * |------BWP0------|------BWP1-----|
   */

  std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());


  band.m_centralFrequency  = centralFrequencyBand;
  band.m_channelBandwidth = bandwidthBand;
  band.m_lowerFrequency = band.m_centralFrequency - band.m_channelBandwidth / 2;
  band.m_higherFrequency = band.m_centralFrequency + band.m_channelBandwidth / 2;

  // Component Carrier 0
  cc0->m_ccId = 0;
  cc0->m_centralFrequency = centralFrequencyCc0;
  cc0->m_channelBandwidth = bandwidthCc0;
  cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
  cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;

  // BWP 0
  bwp0->m_bwpId = 0;
  bwp0->m_centralFrequency = cc0->m_lowerFrequency + cc0->m_channelBandwidth / 4;
  bwp0->m_channelBandwidth = bandwidthCc0Bpw0;
  bwp0->m_lowerFrequency = bwp0->m_centralFrequency - bwp0->m_channelBandwidth / 2;
  bwp0->m_higherFrequency = bwp0->m_centralFrequency + bwp0->m_channelBandwidth / 2;

  cc0->AddBwp (std::move (bwp0));

  // BWP 1
  bwp1->m_bwpId = 1;
  bwp1->m_centralFrequency = cc0->m_higherFrequency - cc0->m_channelBandwidth / 4;
  bwp1->m_channelBandwidth = bandwidthCc0Bpw1;
  bwp1->m_lowerFrequency = bwp1->m_centralFrequency - bwp1->m_channelBandwidth / 2;
  bwp1->m_higherFrequency = bwp1->m_centralFrequency + bwp1->m_channelBandwidth / 2;

  cc0->AddBwp (std::move (bwp1));


  // Add CC to the corresponding operation band.
  band.AddCc (std::move (cc0));

  /********************* END Spectrum division ****************************/

  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  //Set gNB scheduler
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaRR"));

  //gNB Beamforming method
  if (cellScan)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
      idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
    }
  else
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    }

  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1)); // From SL examples
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (2)); // From SL examples
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  //gNB bandwidth part manager setup.
  //The current algorithm multiplexes BWPs depending on the associated bearer QCI
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (0)); // The BWP index is 0 because only one BWP will be installed in the eNB


  //Install gNB and in-network only UEs only in the BWP that will be used for in-network
  //Use BPW 0 to avoid bwpId and vector indexes conflicts and errors during device installation
  uint8_t bwpIdInNet = 0;
  BandwidthPartInfoPtrVector inNetBwp;
  inNetBwp.insert (inNetBwp.end (), band.GetBwpAt (/*CC*/ 0,bwpIdInNet));
  NetDeviceContainer inNetUeNetDev = nrHelper->InstallUeDevice (inNetUeNodes, inNetBwp);
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, inNetBwp);

  //SL UE MAC configuration
  nrHelper->SetUeMacAttribute ("EnableSensing", BooleanValue (false));
  nrHelper->SetUeMacAttribute ("T1", UintegerValue (2));
  nrHelper->SetUeMacAttribute ("T2", UintegerValue (33));
  nrHelper->SetUeMacAttribute ("ActivePoolId", UintegerValue (0));
  nrHelper->SetUeMacAttribute ("ReservationPeriod", TimeValue (MilliSeconds (10)));
  nrHelper->SetUeMacAttribute ("NumSidelinkProcess", UintegerValue (255)); //TODO: It was 4, I increased it because we hit an error where no HARQ processes were available
  nrHelper->SetUeMacAttribute ("EnableBlindReTx", BooleanValue (true));
  nrHelper->SetUeMacAttribute ("SlThresPsschRsrp", IntegerValue (-128));

  //SL BWP manager configuration
  uint8_t bwpIdSl = 1;
  nrHelper->SetBwpManagerTypeId (TypeId::LookupByName ("ns3::NrSlBwpManagerUe"));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_MC_PUSH_TO_TALK", UintegerValue (bwpIdSl));

  //Install both BWPs on UEs doing in-network and SL
  /**
   * BWP bwpIdInNet will be configured by the gNB upon attachment and
   * BWP bwpIdSl will be configured for SL upon installation of SL
   * preconfiguration later in this scenario configuration.
   */
  NetDeviceContainer inNetSlUeNetDev = nrHelper->InstallUeDevice (inNetSlUeNodes, allBwps);

  //Install both BWPs on SL-only UEs
  /**
   * Important! The SL-only UEs will be configured for SL in both BWPs.
   * This was needed to avoid errors with bwpId and vector indexes during device
   * installation, but bwpIdInNet won't be used as all SL traffic is currently
   * hardcoded with GBR_MC_PUSH_TO_TALK QCI in the model, and we have set the BWP
   * manager algorithm to use bwpIdSl BWP for that QCI.
   */
  NetDeviceContainer slUeNetDev = nrHelper->InstallUeDevice (slUeNodes, allBwps );
  std::set<uint8_t> slBwpIdContainer;
  slBwpIdContainer.insert (bwpIdInNet);
  slBwpIdContainer.insert (bwpIdSl);

  //Setup BWPs numerology, Tx Power and pattern
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyCc0Bwp0));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (gNBtotalTxPower));

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }
  for (auto it = inNetUeNetDev.Begin (); it != inNetUeNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
  for (auto it = inNetSlUeNetDev.Begin (); it != inNetSlUeNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
  for (auto it = slUeNetDev.Begin (); it != slUeNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }


  /*Create NrSlHelper which will configure the UEs protocol stack to be ready to
   *perform Sidelink related procedures
   */
  Ptr<NrSlHelper> nrSlHelper = CreateObject <NrSlHelper> ();
  nrSlHelper->SetEpcHelper (epcHelper);

  //Set the SL error model and AMC
  std::string errorModel = "ns3::NrEesmIrT1";
  nrSlHelper->SetSlErrorModel (errorModel);
  nrSlHelper->SetUeSlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));

  //Set the SL scheduler attributes
  nrSlHelper->SetNrSlSchedulerTypeId (NrSlUeMacSchedulerNist::GetTypeId ());
  nrSlHelper->SetUeSlSchedulerAttribute ("FixNrSlMcs", BooleanValue (true));
  nrSlHelper->SetUeSlSchedulerAttribute ("InitialNrSlMcs", UintegerValue (14));

  //Configure UEs doing in-network and SL for SL
  std::set<uint8_t> slBwpIdContainerInNetSlUes;
  slBwpIdContainerInNetSlUes.insert (bwpIdSl);   //Only in the SL BWP for these UEs
  nrSlHelper->PrepareUeForSidelink (inNetSlUeNetDev, slBwpIdContainerInNetSlUes);

  //Configure SL-only UEs for SL
  nrSlHelper->PrepareUeForSidelink (slUeNetDev, slBwpIdContainer);

  /***SL IEs configuration **/

  //SlResourcePoolNr IE
  LteRrcSap::SlResourcePoolNr slResourcePoolNr;
  //get it from pool factory
  Ptr<NrSlCommPreconfigResourcePoolFactory> ptrFactory = Create<NrSlCommPreconfigResourcePoolFactory> ();
  //Configure specific parameters of interest:
  std::vector <std::bitset<1> > slBitmap = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  ptrFactory->SetSlTimeResources (slBitmap);
  ptrFactory->SetSlSensingWindow (100); // T0 in ms
  ptrFactory->SetSlSelectionWindow (5);
  ptrFactory->SetSlFreqResourcePscch (10); // PSCCH RBs
  ptrFactory->SetSlSubchannelSize (10);
  ptrFactory->SetSlMaxNumPerReserve (3);
  //Once parameters are configured, we can create the pool
  LteRrcSap::SlResourcePoolNr pool = ptrFactory->CreatePool ();
  slResourcePoolNr = pool;

  //Configure the SlResourcePoolConfigNr IE, which hold a pool and its id
  LteRrcSap::SlResourcePoolConfigNr slresoPoolConfigNr;
  slresoPoolConfigNr.haveSlResourcePoolConfigNr = true;
  //Pool id, ranges from 0 to 15
  uint16_t poolId = 0;
  LteRrcSap::SlResourcePoolIdNr slResourcePoolIdNr;
  slResourcePoolIdNr.id = poolId;
  slresoPoolConfigNr.slResourcePoolId = slResourcePoolIdNr;
  slresoPoolConfigNr.slResourcePool = slResourcePoolNr;

  //Configure the SlBwpPoolConfigCommonNr IE, which hold an array of pools
  LteRrcSap::SlBwpPoolConfigCommonNr slBwpPoolConfigCommonNr;
  //Array for pools, we insert the pool in the array as per its poolId
  slBwpPoolConfigCommonNr.slTxPoolSelectedNormal [slResourcePoolIdNr.id] = slresoPoolConfigNr;

  //Configure the BWP IE
  LteRrcSap::Bwp bwp;
  bwp.numerology = numerologyCc0Bwp1;
  bwp.symbolsPerSlots = 14;
  bwp.rbPerRbg = 1;
  bwp.bandwidth = bandwidthCc0Bpw1;

  //Configure the SlBwpGeneric IE
  LteRrcSap::SlBwpGeneric slBwpGeneric;
  slBwpGeneric.bwp = bwp;
  slBwpGeneric.slLengthSymbols = LteRrcSap::GetSlLengthSymbolsEnum (14);
  slBwpGeneric.slStartSymbol = LteRrcSap::GetSlStartSymbolEnum (0);

  //Configure the SlBwpConfigCommonNr IE
  LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommonNr;
  slBwpConfigCommonNr.haveSlBwpGeneric = true;
  slBwpConfigCommonNr.slBwpGeneric = slBwpGeneric;
  slBwpConfigCommonNr.haveSlBwpPoolConfigCommonNr = true;
  slBwpConfigCommonNr.slBwpPoolConfigCommonNr = slBwpPoolConfigCommonNr;

  //Configure the SlFreqConfigCommonNr IE, which hold the array to store
  //the configuration of all Sidelink BWP (s).
  LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNr;
  //Array for BWPs. Here we will iterate over the BWPs, which
  //we want to use for SL.
  for (const auto &it:slBwpIdContainer)
    {
      // it is the BWP id
      slFreConfigCommonNr.slBwpList [it] = slBwpConfigCommonNr;
    }

  //Configure the TddUlDlConfigCommon IE
  LteRrcSap::TddUlDlConfigCommon tddUlDlConfigCommon;
  tddUlDlConfigCommon.tddPattern = pattern;

  //Configure the SlPreconfigGeneralNr IE
  LteRrcSap::SlPreconfigGeneralNr slPreconfigGeneralNr;
  slPreconfigGeneralNr.slTddConfig = tddUlDlConfigCommon;

  //Configure the SlUeSelectedConfig IE
  LteRrcSap::SlUeSelectedConfig slUeSelectedPreConfig;
  slUeSelectedPreConfig.slProbResourceKeep = 0;
  //Configure the SlPsschTxParameters IE
  LteRrcSap::SlPsschTxParameters psschParams;
  psschParams.slMaxTxTransNumPssch = 5;
  //Configure the SlPsschTxConfigList IE
  LteRrcSap::SlPsschTxConfigList pscchTxConfigList;
  pscchTxConfigList.slPsschTxParameters [0] = psschParams;
  slUeSelectedPreConfig.slPsschTxConfigList = pscchTxConfigList;

  /*
   * Finally, configure the SidelinkPreconfigNr This is the main structure
   * that needs to be communicated to NrSlUeRrc class
   */
  LteRrcSap::SidelinkPreconfigNr slPreConfigNr;
  slPreConfigNr.slPreconfigGeneral = slPreconfigGeneralNr;
  slPreConfigNr.slUeSelectedPreConfig = slUeSelectedPreConfig;
  slPreConfigNr.slPreconfigFreqInfoList [0] = slFreConfigCommonNr;

  //Communicate the above pre-configuration to the NrSlHelper
  //For SL-only UEs
  nrSlHelper->InstallNrSlPreConfiguration (slUeNetDev, slPreConfigNr);

  //For the UEs doing both in-network and SL
  //We need to modify some parameters to configure *only* BWP1 on these UEs for
  //SL and avoid MAC problems
  LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNrInNetSlUes;
  slFreConfigCommonNrInNetSlUes.slBwpList [bwpIdSl] = slBwpConfigCommonNr;

  LteRrcSap::SidelinkPreconfigNr slPreConfigNrInNetSlUes;
  slPreConfigNrInNetSlUes.slPreconfigGeneral = slPreconfigGeneralNr;
  slPreConfigNrInNetSlUes.slUeSelectedPreConfig = slUeSelectedPreConfig;
  slPreConfigNrInNetSlUes.slPreconfigFreqInfoList [0] = slFreConfigCommonNrInNetSlUes;

  nrSlHelper->InstallNrSlPreConfiguration (inNetSlUeNetDev, slPreConfigNrInNetSlUes);

  /***END SL IEs configuration **/

  //Set random streams
  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (inNetUeNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (inNetSlUeNetDev, randomStream);
  randomStream += nrSlHelper->AssignStreams (inNetSlUeNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (slUeNetDev, randomStream);
  randomStream += nrSlHelper->AssignStreams (slUeNetDev, randomStream);

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Configure in-network UEs
  internet.Install (inNetUeNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (inNetUeNetDev));
  // Set the default gateway for the in-network UEs
  for (uint32_t j = 0; j < inNetUeNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (inNetUeNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  //Attach in-network UEs to the closest gNB
  nrHelper->AttachToClosestEnb (inNetUeNetDev, enbNetDev);

  // Configure UEs doing both in-network and SL
  internet.Install (inNetSlUeNodes);
  Ipv4InterfaceContainer ueIpIfaceInNetSlUes;
  ueIpIfaceInNetSlUes = epcHelper->AssignUeIpv4Address (NetDeviceContainer (inNetSlUeNetDev));
  std::vector<Ipv4Address> inNetSlUesIpv4AddressVector (inNetSlUeNum);

  for (uint32_t u = 0; u < inNetSlUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (inNetSlUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      inNetSlUesIpv4AddressVector [u] = inNetSlUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    }

  //Attach UEs doing in-netwrok and SL to the closest gNB
  nrHelper->AttachToClosestEnb (inNetSlUeNetDev, enbNetDev);

  // Configure SL UEs
  internet.Install (slUeNodes);
  Ipv4InterfaceContainer ueIpIfaceSl;
  ueIpIfaceSl = epcHelper->AssignUeIpv4Address (NetDeviceContainer (slUeNetDev));
  std::vector<Ipv4Address> slIpv4AddressVector (slUeNum);

  for (uint32_t u = 0; u < slUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (slUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      slIpv4AddressVector [u] = slUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    }

  /******** Configure ProSe layer in the SL UEs **********/
  //Create ProSe helper
  Ptr<NrSlProseHelper> nrSlProseHelper = CreateObject <NrSlProseHelper> ();
  // Install ProSe layer and corresponding SAPs in the UES
  nrSlProseHelper->PrepareUesForProse (inNetSlUeNetDev);
  nrSlProseHelper->PrepareUesForProse (slUeNetDev);

  //Configure ProSe Unicast parameters. At the moment it only instruct the MAC
  //layer (and PHY therefore) to monitor packets directed the UE's own Layer 2 ID
  nrSlProseHelper->PrepareUesForUnicast (inNetSlUeNetDev);
  nrSlProseHelper->PrepareUesForUnicast (slUeNetDev);

  /*
   * Setup the start of the ProSe direct link establishment procedure
   * (with the 'Real' protocol, PC5-S messages are exchanged over the SL)
   * First UE on the function call will be the initiating UE (UE i),
   * which starts the procedure, and it is interested in establish a direct
   * link with the following j UEs, which will be the target UEs
   */
  NS_LOG_INFO ("Configuring unicast direct links on SL-only UEs..." );
  for (uint32_t i = 0; i < slUeNodes.GetN () - 1; ++i)
    {
      for (uint32_t j = i + 1; j < slUeNodes.GetN (); ++j)
        {
          nrSlProseHelper->EstablishRealDirectLink (startDirLinkTime,
                                                    slUeNetDev.Get (i), slIpv4AddressVector [i],  // Initiating UE
                                                    slUeNetDev.Get (j), slIpv4AddressVector [j]); // Target UE
          NS_LOG_INFO ("Initiating UE nodeId " << i << " target UE nodeId " << j );

        }
    }
  NS_LOG_INFO ("Configuring unicast direct links on UEs doing in-network and SL..." );
  for (uint32_t i = 0; i < slUeNodes.GetN (); ++i)
    {
      for (uint32_t j = 0; j < inNetSlUeNetDev.GetN (); ++j)
        {
          nrSlProseHelper->EstablishRealDirectLink (startDirLinkTime,
                                                    slUeNetDev.Get (i), slIpv4AddressVector [i],            // Initiating UE
                                                    inNetSlUeNetDev.Get (j), inNetSlUesIpv4AddressVector [j]);    // Target UE
          NS_LOG_INFO ("Initiating UE nodeId " << i << " target UE nodeId " << j );

        }
    }

  /******** END Configure ProSe layer in the SL UEs **********/


  /********* In-network applications configuration ******/
  // install UDP applications
  uint16_t dlPort = 1234;
  uint16_t ulPort = dlPort + gNbNum * inNetUeNumPerGnb + 1;
  ApplicationContainer clientApps, serverApps;

  std::cout << "In-network traffic flows: " << std::endl;
  for (uint32_t u = 0; u < inNetUeNodes.GetN (); ++u)
    {
      //DL traffic
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      serverApps.Add (dlPacketSinkHelper.Install (inNetUeNodes.Get (u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("PacketSize", UintegerValue (packetSizeDlUl));
      dlClient.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaDl)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      clientApps.Add (dlClient.Install (remoteHost));

      std::cout << "DL: " << remoteHostAddr << " --> " << ueIpIface.GetAddress (u) << ":" << dlPort <<
        " start time: " << inNetTrafficStartTime  << " s, end time: " << simTime << " s" << std::endl;

      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      pfDl.localPortStart = dlPort;
      pfDl.localPortEnd = dlPort;
      ++dlPort;
      tftDl->Add (pfDl);

      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;

      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (u), bearerDl, tftDl);

      //UL traffic
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("PacketSize", UintegerValue (packetSizeDlUl));
      ulClient.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUl)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      clientApps.Add (ulClient.Install (inNetUeNodes.Get (u)));

      std::cout << "UL: " << ueIpIface.GetAddress (u)  << " --> " << remoteHostAddr << ":"  << ulPort  <<
        " start time: " << inNetTrafficStartTime  << " s, end time: " << simTime << " s" << std::endl;

      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remotePortStart = ulPort;
      pfUl.remotePortEnd = ulPort;
      ++ulPort;
      tftUl->Add (pfUl);

      enum EpsBearer::Qci qUl;

      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (u), bearerUl, tftUl);
    }

  //In-network traffic for UEs doing in-network and SL
  for (uint32_t u = 0; u < inNetSlUeNodes.GetN (); ++u)
    {
      //DL traffic
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      serverApps.Add (dlPacketSinkHelper.Install (inNetSlUeNodes.Get (u)));

      UdpClientHelper dlClient (ueIpIfaceInNetSlUes.GetAddress (u), dlPort);
      dlClient.SetAttribute ("PacketSize", UintegerValue (packetSizeDlUl));
      dlClient.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaDl)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      clientApps.Add (dlClient.Install (remoteHost));

      std::cout << "DL: " << remoteHostAddr << " --> " << ueIpIfaceInNetSlUes.GetAddress (u) << ":" << dlPort <<
        " start time: " << inNetTrafficStartTime  << " s, end time: " << simTime << " s" << std::endl;

      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      pfDl.localPortStart = dlPort;
      pfDl.localPortEnd = dlPort;
      ++dlPort;
      tftDl->Add (pfDl);

      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;

      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (inNetSlUeNetDev.Get (u), bearerDl, tftDl);

      //UL traffic
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("PacketSize", UintegerValue (packetSizeDlUl));
      ulClient.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUl)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      clientApps.Add (ulClient.Install (inNetSlUeNodes.Get (u)));

      std::cout << "UL: " << ueIpIfaceInNetSlUes.GetAddress (u)  << " --> " << remoteHostAddr << ":"  << ulPort  <<
        " start time: " << inNetTrafficStartTime  << " s, end time: " << simTime << " s" << std::endl;

      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remotePortStart = ulPort;
      pfUl.remotePortEnd = ulPort;
      ++ulPort;
      tftUl->Add (pfUl);

      enum EpsBearer::Qci qUl;

      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (inNetSlUeNetDev.Get (u), bearerUl, tftUl);
    }

  serverApps.Start (Seconds (inNetTrafficStartTime));
  clientApps.Start (Seconds (inNetTrafficStartTime));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime));

  /********* END In-network applications configuration ******/



  /******************* SL application configuration *****************************/
  /* - Client app: OnOff application configured to generate CBR traffic.
   *               Installed in the initiating UE of each link by default, and
   *               in target UE if 'bidirectional' flag is True.
   * - Server app: PacketSink application to consume the received traffic.
   *               Installed in all UEs
   */
  // Random variable to randomize a bit start times of the client applications
  //to avoid simulation artifacts of all the TX UEs transmitting at the same time.
  Ptr<UniformRandomVariable> startTimeRnd = CreateObject<UniformRandomVariable> ();
  startTimeRnd->SetAttribute ("Min", DoubleValue (0));
  startTimeRnd->SetAttribute ("Max", DoubleValue (0.10));
  uint16_t slPort = 8000;

  std::string dataRateString  = std::to_string (dataRateSl) + "kb/s";
  ApplicationContainer slClientApps;
  std::cout << "Sidelink traffic flows: " << std::endl;
  for (uint32_t i = 0; i < slUeNodes.GetN () - 1; ++i)
    {
      for (uint32_t j = i + 1; j < slUeNodes.GetN (); ++j)
        {
          OnOffHelper sidelinkClient ("ns3::UdpSocketFactory",
                                      InetSocketAddress (slIpv4AddressVector [j], slPort)); //Towards UE j
          sidelinkClient.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
          sidelinkClient.SetConstantRate (DataRate (dataRateString), packetSizeSl);
          ApplicationContainer app = sidelinkClient.Install (slUeNodes.Get (i)); // Installed in UE i
          Time appStart = slTrafficStartTime + Seconds (startTimeRnd->GetValue ());
          app.Start (appStart);
          slClientApps.Add (app);
          NS_LOG_INFO ("OnOff application installed in UE nodeId " << i << " srcIp " << slIpv4AddressVector [i] <<
                       " towards UE nodeId " << j << " dstIp " << slIpv4AddressVector [j] );
          std::cout << "SL: " << slIpv4AddressVector [i] << " -> " << slIpv4AddressVector [j] <<
            " start time: " << appStart.GetSeconds () << " s, end time: " << simTime  << " s" << std::endl;

          if (bidirectionalSlTraffic)
            {
              OnOffHelper sidelinkClient ("ns3::UdpSocketFactory",
                                          InetSocketAddress (slIpv4AddressVector [i], slPort)); // Towards UE i
              sidelinkClient.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
              sidelinkClient.SetConstantRate (DataRate (dataRateString), packetSizeSl);
              ApplicationContainer app = sidelinkClient.Install (slUeNodes.Get (j)); // Installed in UE j
              Time appStart = slTrafficStartTime + Seconds (startTimeRnd->GetValue ());
              app.Start (appStart);
              slClientApps.Add (app);
              NS_LOG_INFO ("OnOff application installed in UE nodeId " << j << " srcIp " << slIpv4AddressVector [j] <<
                           " towards UE nodeId " << i << " dstIp " << slIpv4AddressVector [i] );
              std::cout << "SL: " << slIpv4AddressVector [j] << " -> " << slIpv4AddressVector [i] <<
                " start time: " << appStart.GetSeconds ()  << " s, end time: " << simTime << " s" << std::endl;
            }
        }
    }

  //SL traffic for UEs doing in-network and SL
  for (uint32_t i = 0; i < slUeNodes.GetN (); ++i)
    {
      for (uint32_t j = 0; j < inNetSlUeNodes.GetN (); ++j)
        {
          OnOffHelper sidelinkClient ("ns3::UdpSocketFactory",
                                      InetSocketAddress (inNetSlUesIpv4AddressVector [j], slPort));   //Towards inNetSlUe j
          sidelinkClient.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
          sidelinkClient.SetConstantRate (DataRate (dataRateString), packetSizeSl);
          ApplicationContainer app = sidelinkClient.Install (slUeNodes.Get (i));   // Installed in slUe i
          Time appStart = slTrafficStartTime + Seconds (startTimeRnd->GetValue ());
          app.Start (appStart);
          slClientApps.Add (app);
          NS_LOG_INFO ("OnOff application installed in UE <nodeId> " << i << " srcIp " << slIpv4AddressVector [i] <<
                       " towards UE <nodeId> " << j << " dstIp " << inNetSlUesIpv4AddressVector [j] );
          std::cout << "SL: " << slIpv4AddressVector [i] << " -> " << inNetSlUesIpv4AddressVector [j] <<
            " start time: " << appStart.GetSeconds () << " s, end time: " << simTime  << " s" << std::endl;

          if (bidirectionalSlTraffic)
            {
              OnOffHelper sidelinkClient ("ns3::UdpSocketFactory",
                                          InetSocketAddress (slIpv4AddressVector [i], slPort));   // Towards slUe i
              sidelinkClient.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
              sidelinkClient.SetConstantRate (DataRate (dataRateString), packetSizeSl);
              ApplicationContainer app = sidelinkClient.Install (inNetSlUeNodes.Get (j));   // Installed in inNetSlUe j
              Time appStart = slTrafficStartTime + Seconds (startTimeRnd->GetValue ());
              app.Start (appStart);
              slClientApps.Add (app);
              NS_LOG_INFO ("OnOff application installed in UE <nodeId> " << j << " srcIp " << inNetSlUesIpv4AddressVector [j] <<
                           " towards UE <nodeId> " << i << " dstIp " << slIpv4AddressVector [i] );
              std::cout << "SL: " << inNetSlUesIpv4AddressVector [j] << " -> " << slIpv4AddressVector [i] <<
                " start time: " << appStart.GetSeconds ()  << " s, end time: " << simTime << " s" << std::endl;

            }
        }
    }
  slClientApps.Stop (Seconds (simTime));

  ApplicationContainer slServerApps;
  PacketSinkHelper sidelinkSink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), slPort));
  sidelinkSink.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
  slServerApps = sidelinkSink.Install (NodeContainer (slUeNodes, inNetSlUeNodes));  // Installed in all UEs
  slServerApps.Start (Seconds (1.0));
  slServerApps.Stop (Seconds (simTime));

  /******************** End SL application configuration ************************/


  /************ SL traces database setup *************************************/

  std::string exampleName = simTag + "-" + "nr-prose-network-same-ue";
  SQLiteOutput db (outputDir + exampleName + "SLtraces.db", exampleName);

  UeMacPscchTxOutputStats pscchStats;
  pscchStats.SetDb (&db, "pscchTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPscchScheduling",
                                 MakeBoundCallback (&NotifySlPscchScheduling, &pscchStats));

  UeMacPsschTxOutputStats psschStats;
  psschStats.SetDb (&db, "psschTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPsschScheduling",
                                 MakeBoundCallback (&NotifySlPsschScheduling, &psschStats));


  UePhyPscchRxOutputStats pscchPhyStats;
  pscchPhyStats.SetDb (&db, "pscchRxUePhy");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/SpectrumPhy/RxPscchTraceUe",
                                 MakeBoundCallback (&NotifySlPscchRx, &pscchPhyStats));

  UePhyPsschRxOutputStats psschPhyStats;
  psschPhyStats.SetDb (&db, "psschRxUePhy");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/SpectrumPhy/RxPsschTraceUe",
                                 MakeBoundCallback (&NotifySlPsschRx, &psschPhyStats));

  UeToUePktTxRxOutputStats pktStats;
  pktStats.SetDb (&db, "pktTxRx");

  for (uint16_t ac = 0; ac < slClientApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  slClientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      slClientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&UePacketTraceDb, &pktStats, slClientApps.Get (ac)->GetNode (), localAddrs));
    }

  for (uint16_t ac = 0; ac < slServerApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  slServerApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      slServerApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&UePacketTraceDb, &pktStats, slServerApps.Get (ac)->GetNode (), localAddrs));
    }

  /************ END SL traces database setup *************************************/

  nrHelper->EnableTraces ();

  //Configure FlowMonitor to get traffic flow statistics
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (inNetUeNodes);
  endpointNodes.Add (slUeNodes);
  endpointNodes.Add (inNetSlUeNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  //Run simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  //SL database dump
  pktStats.EmptyCache ();
  pscchStats.EmptyCache ();
  psschStats.EmptyCache ();
  pscchPhyStats.EmptyCache ();
  psschPhyStats.EmptyCache ();

  //Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }

      double appDuration = 0;
      if ( t.destinationPort == 8000) //SL
        {
          appDuration = simTime - slTrafficStartTime.GetSeconds ();
          //TODO Some inaccuracy is expected due to randomization of start time.
        }
      else
        {
          appDuration = simTime - inNetTrafficStartTime;
        }

      outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") " << protoStream.str () << "\n";
      outFile << "  Tx Packets: " << i->second.txPackets << "\n";
      outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps\n";
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {


          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps\n";
          outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          outFile << "  Throughput:  0 Mbps\n";
          outFile << "  Mean delay:  0 ms\n";
          outFile << "  Mean jitter: 0 ms\n";
        }
    }
  outFile.close ();

  std::cout << "Simulation done!"  << std::endl << "Traffic flows statistics: " << std::endl;
  std::ifstream f (filename.c_str ());
  if (f.is_open ())
    {
      std::cout << f.rdbuf ();
    }

  Simulator::Destroy ();
  return 0;
}


