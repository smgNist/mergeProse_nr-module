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
 * \ingroup examples
 * \file milcom-2023.cc
 *
 *
 * \code{.unparsed}
$ ./ns3 run "milcom-2023 --Help"
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
#include <ns3/nr-sl-pc5-signalling-header.h>
#include <ns3/nr-sl-ue-prose.h>
#include <ns3/epc-ue-nas.h>
#include "ns3/gnuplot.h"
#include <sqlite3.h>

#ifdef HAS_NETSIMULYZER
#include <ns3/netsimulyzer-module.h>
#endif

//Dependency of these nr-v2x-examples classes for SL statistics
#include "../nr-v2x-examples/ue-mac-pscch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-mac-pssch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pscch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pssch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-to-ue-pkt-txrx-output-stats.h"
#include "../nr-v2x-examples/ue-rlc-rx-output-stats.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Milcom2023");

/**************** Methods for tracing SL using databases *********************/
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
void NotifySlRlcPduRx (UeRlcRxOutputStats *stats, uint64_t imsi, uint16_t rnti, uint16_t txRnti, uint8_t lcid, uint32_t rxPduSize, double delay)
{
  stats->Save (imsi, rnti, txRnti, lcid, rxPduSize, delay);
}
void
PacketTraceDb (UeToUePktTxRxOutputStats *stats, Ptr<Node> node, const Address &localAddrs,
               std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
               const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  uint32_t nodeId = node->GetId ();
  uint64_t imsi;
  if (node->GetDevice (0)->GetObject<NrUeNetDevice> () != nullptr)
    {
      imsi = node->GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();
    }
  else
    {
      //It is the Remote Host node
      imsi = 0;
    }
  uint32_t seq = seqTsSizeHeader.GetSeq ();
  uint32_t pktSize = p->GetSize () + seqTsSizeHeader.GetSerializedSize ();

  stats->Save (txRx, localAddrs, nodeId, imsi, pktSize, srcAddrs, dstAddrs, seq);

}

/************** END Methods for tracing SL using databases *******************/

/*
 * \brief Trace sink function for logging transmission and reception of PC5
 *        signaling (PC5-S) messages
 *
 * \param stream the output stream wrapper where the trace will be written
 * \param node the pointer to the UE node
 * \param srcL2Id the L2 ID of the UE sending the PC5-S packet
 * \param dstL2Id the L2 ID of the UE receiving the PC5-S packet
 * \param isTx flag that indicates if the UE is transmitting the PC5-S packet
 * \param p the PC5-S packet
 */
void
TraceSinkPC5SignallingPacketTrace (Ptr<OutputStreamWrapper> stream,
                                   Ptr<Node> node,
                                   uint32_t srcL2Id, uint32_t dstL2Id, bool isTx, Ptr<Packet> p)
{
  NrSlPc5SignallingMessageType pc5smt;
  p->PeekHeader (pc5smt);
  *stream->GetStream () << Simulator::Now ().GetSeconds ()
                            << "\t" << node->GetId ();
  if (isTx)
    {
      *stream->GetStream () << "\t" << "TX";
    }
  else
    {
      *stream->GetStream () << "\t" << "RX";
    }
  *stream->GetStream () << "\t" << srcL2Id << "\t" << dstL2Id << "\t" << pc5smt.GetMessageName ();
  *stream->GetStream () << std::endl;
}


std::map<std::string, uint32_t> g_relayNasPacketCounter;
/*
 * \brief Trace sink function for logging reception of data packets in the NAS
 *        layer by UE(s) acting as relay UE
 *
 * \param stream the output stream wrapper where the trace will be written
 * \param node the pointer to the UE node
 * \param nodeIp the IP of the relay UE
 * \param srcIp the IP of the node sending the packet
 * \param dstIp the IP of the node that would be receiving the packet
 * \param srcLink the link from which the relay UE received the packet (UL, DL, or SL)
 * \param dstLink the link towards which the relay routed the packet (UL, DL, or SL)
 * \param p the packet
 */
void
TraceSinkRelayNasRxPacketTrace (Ptr<OutputStreamWrapper> stream,
                                Ptr<Node> node,
                                Ipv4Address nodeIp, Ipv4Address srcIp, Ipv4Address dstIp,
                                std::string srcLink, std::string dstLink, Ptr<Packet> p)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds ()
                                        << "\t" << node->GetId ()
                                        << "\t" << nodeIp
                                        << "\t" << srcIp
                                        << "\t" << dstIp
                                        << "\t" << srcLink
                                        << "\t" << dstLink
                                        << std::endl;
  std::ostringstream  oss;
  oss << nodeIp << "      " << srcIp << "->" << dstIp << "      " << srcLink << "->" << dstLink;
  std::string mapKey = oss.str ();
  auto it = g_relayNasPacketCounter.find (mapKey);
  if (it == g_relayNasPacketCounter.end ())
    {
      g_relayNasPacketCounter.insert (std::pair < std::string, uint32_t> (mapKey, 1));
    }
  else
    {
      it->second += 1;
    }
}

/*
 * Structure to keep track of the transmission time of the packets at the
 * application layer. Used to calculate packet delay.
 */
struct PacketWithRxTimestamp
{
  Ptr<const Packet> p;
  Time txTimestamp;
};

/*
 * Map to store received packets and reception timestamps at the application
 * layer. Used to calculate packet delay at the application layer.
 */
std::map<std::string, PacketWithRxTimestamp> g_packetsForDelayCalc;
uint32_t g_nTxPackets {0}; //!< Global variable to count TX packets
uint32_t g_nRxPackets {0}; //!< Global variable to count RX packets
std::list <double> g_delays; //!< Global list to store packet delays upon RX


/*
 * \brief Trace sink function for logging the transmitted data packets and their
 * 		  corresponding transmission timestamp at the application layer
 *
 * \param localAddrs the IP address of the node transmitting the packet
 * \param p the packet
 * \param srcAddrs the source IP address in the packet
 * \param dstAddrs the destination IP address in the packet
 * \param seqTsSizeHeader the header containing the sequence number of the packet
 */
void
TxPacketTrace (const Address &localAddrs, Ptr<const Packet> p, const Address &srcAddrs,
               const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  g_nTxPackets ++;
  std::ostringstream  oss;
  oss << Ipv4Address::ConvertFrom (localAddrs)
  << "->"
  << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ()
  << "("
  << seqTsSizeHeader.GetSeq ()
  << ")";
  std::string mapKey = oss.str ();
  PacketWithRxTimestamp mapValue;
  mapValue.p = p;
  mapValue.txTimestamp = Simulator::Now ();
  g_packetsForDelayCalc.insert (std::pair < std::string, PacketWithRxTimestamp> (mapKey, mapValue));

}

/*
 * \brief Trace sink function for calculating and tracing the packet delay upon
 *        reception at the application layer
 *
 * \param stream the output stream wrapper where the trace will be written
 * \param node the node receiving the packet
 * \param localAddrs the IP address of the node receiving the packet
 * \param p the packet
 * \param srcAddrs the source IP address in the packet
 * \param dstAddrs the destination IP address in the packet
 * \param seqTsSizeHeader the header containing the sequence number of the packet
 */
void
RxPacketTrace (Ptr<OutputStreamWrapper> stream, Ptr<Node> node, const Address &localAddrs,
               Ptr<const Packet> p, const Address &srcAddrs,
               const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  g_nRxPackets ++;
  double delay = 0.0;
  std::ostringstream  oss;
  oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
                      << "->"
                      << Ipv4Address::ConvertFrom (localAddrs)
  << "("
  << seqTsSizeHeader.GetSeq ()
  << ")";
  std::string mapKey = oss.str ();
  auto it = g_packetsForDelayCalc.find (mapKey);
  if (it == g_packetsForDelayCalc.end ())
    {
      NS_FATAL_ERROR ("Rx packet not found?!");
    }
  else
    {
      delay = Simulator::Now ().GetSeconds () * 1000.0 - it->second.txTimestamp.GetSeconds () * 1000.0;
      g_packetsForDelayCalc.erase (mapKey);
      g_delays.push_back (delay);
    }
  if (stream != nullptr)
    {
      *stream->GetStream () << Simulator::Now ().GetSeconds ()
                                            << "\t" << node->GetId ()
                                            << "\t" << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
                                            << "\t" << Ipv4Address::ConvertFrom (localAddrs)
      << "\t" << seqTsSizeHeader.GetSeq ()
      << "\t" << delay
      << std::endl;
    }
}


/**
 * \brief Function that generates a gnuplot script file that can be used to
 *        plot the topology of the scenario access network (gNB, in-network
 *        only UEs, Relay UEs and Remote UEs)
 *
 * \param fileNameWithNoExtension the name of the output file
 * \param gNbNode the gNB node container
 * \param vehicleUeNodes
 * \param soldierUeNodes
 * \param platoonSideLength
 * \param squadSideLength
 */
void


GenerateTopologyPlotFile (std::string fileNameWithNoExtension, NodeContainer gNbNode,
                          NodeContainer vehicleUeNodes, NodeContainer soldierUeNodes,
                          double platoonSideLength, double squadSideLength, double inCoverageRadius)
{
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string gnuplotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "Topology (Labels = Node IDs)";

  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);
  plot.SetTerminal ("png size 1024,1024");
  plot.SetLegend ("X (m)", "Y (m)"); //These are the axis, not the legend
  std::ostringstream plotExtras;
  plotExtras << "set xrange [-" << 0 << ":+" << 1.1 * platoonSideLength << "]" << std::endl;
  plotExtras << "set yrange [-" << 0 << ":+" << 1.1 * platoonSideLength << "]" << std::endl;
  plotExtras << "set linetype 1 pt 3 ps 2 " << std::endl;
  plotExtras << "set linetype 2 lc rgb \"green\" pt 2 ps 2" << std::endl;
  plotExtras << "set linetype 3 pt 1 ps 2" << std::endl;
  plotExtras << "set obj rect from "<< 0 << "," << 0 << " to " << platoonSideLength <<","<< platoonSideLength
      << " fs empty border rgb \"yellow\"" << std::endl;
  plotExtras << "set style rect fs empty border rgb \"green\"" << std::endl;

  plotExtras << "set object circle at " << gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().x <<
      "," << gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().y << " size first "<< inCoverageRadius
      <<" fc rgb \"red\" "<< std::endl;


  //Create squad areas
  for (uint32_t ryIdx = 0; ryIdx < vehicleUeNodes.GetN (); ryIdx++)
    {
      double x = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      plotExtras << "set obj rect from "<< x << "," << y
          << " to " << x + squadSideLength <<","<< y + squadSideLength<< std::endl;
    }

  plot.AppendExtra (plotExtras.str ());

  //gNB
  Gnuplot2dDataset datasetEnodeB;
  datasetEnodeB.SetTitle ("gNodeB");
  datasetEnodeB.SetStyle (Gnuplot2dDataset::POINTS);

  double x = gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().x;
  double y = gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().y;
  std::ostringstream strForLabel;
  strForLabel << "set label \"" << gNbNode.Get (0)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"black\" center front offset 0,1";
  plot.AppendExtra (strForLabel.str ());
  datasetEnodeB.Add (x, y);
  plot.AddDataset (datasetEnodeB);

  //Vehicle UEs
  Gnuplot2dDataset datasetRelays;
  datasetRelays.SetTitle ("Vehicle UEs");
  datasetRelays.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t ryIdx = 0; ryIdx < vehicleUeNodes.GetN (); ryIdx++)
    {
      double x = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << vehicleUeNodes.Get (ryIdx)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"black\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetRelays.Add (x, y);
    }
  plot.AddDataset (datasetRelays);

  //Soldier UEs
  Gnuplot2dDataset datasetRemotes;
  datasetRemotes.SetTitle ("Soldier UEs");
  datasetRemotes.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t rmIdx = 0; rmIdx < soldierUeNodes.GetN (); rmIdx++)
    {
      double x = soldierUeNodes.Get (rmIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = soldierUeNodes.Get (rmIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << soldierUeNodes.Get (rmIdx)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"black\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetRemotes.Add (x, y);
    }
  plot.AddDataset (datasetRemotes);

  std::ofstream plotFile (gnuplotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();
}


int
main (int argc, char *argv[])
{
  //Common configuration
  //  double centralFrequencyBand = 5.89e9; // band n47 (From SL examples)
  double centralFrequencyBand = 795e6; // 795 MHz band n14
  double bandwidthBand = 40e6; //40 MHz
  //  double centralFrequencyCc0 = 5.89e9;
  double centralFrequencyCc0 = 795e6; // 795 MHz band n14
  double bandwidthCc0 = bandwidthBand;
  std::string pattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|";
  double bandwidthCc0Bpw0 = bandwidthCc0 / 2;
  double bandwidthCc0Bpw1 = bandwidthCc0 / 2;
  double ueTxPower = 23; //dBm

  //In-network devices configuration
  //  uint16_t numerologyCc0Bwp0 = 2; // BWP0 will be used for the in-network
  uint16_t numerologyCc0Bwp0 = 0; // BWP0 will be used for the in-network

  double gNBtotalTxPower = 32; // dBm
  bool cellScan = false;  // Beamforming method.
  double beamSearchAngleStep = 10.0; // Beamforming parameter.

  //Sidelink configuration
  uint16_t numerologyCc0Bwp1 = 2; //(From SL examples)  BWP1 will be used for SL
  Time startRelayConnTime = Seconds (2.0); //Time to start the U2N relay connection establishment
  bool enableSensing = false;

  //Topology
  double gNbHeight = 10; //meters
  double vehicleUeHeight = 1.5; //meters
  double soldierUeHeight = 1.5; //meters
  uint16_t nSquads = 1;
  uint16_t nSoldiersPerSquad = 5;
  double platoonSideLength = 1000.0; //meters
  double squadSideLength = 200.0; //meters
  double inCoverageRadius = 500.0; //meters
  bool dropVehiclesCellEdge = true;
  bool soldierAttachToSquadVehicle = true;
  bool forceSoldiersUeOoc = true;

  bool rangeTest = false;

  //Traffic
  Time timeStartTraffic = Seconds (5.0);
  Time durationTraffic = Seconds (10.0);
  uint16_t onOffPacketSize = 60; //bytes
  double onOffDataRate = 24.0; //kbps
  bool ulTraffic = true;
  bool dlTraffic = false;
  bool relayUesTraffic = false;
  bool inNetTraffic = false;

  //Simulation configuration
  std::string exampleName = "milcom-2023";
  bool writeTraces = false;

  CommandLine cmd;
  cmd.AddValue ("nSquads", "", nSquads);
  cmd.AddValue ("nSoldiersPerSquad", "", nSoldiersPerSquad);
  cmd.AddValue ("platoonSideLength", "", platoonSideLength);
  cmd.AddValue ("squadSideLength", "", squadSideLength);
  cmd.AddValue ("inCoverageRadius", "", inCoverageRadius);
  cmd.AddValue ("durationTraffic", "", durationTraffic);
  cmd.AddValue ("rangeTest", "", rangeTest);

  cmd.AddValue ("onOffPacketSize", "", onOffPacketSize);
  cmd.AddValue ("onOffDataRate", "", onOffDataRate);
  cmd.AddValue ("ulTraffic", "", ulTraffic);
  cmd.AddValue ("dlTraffic", "", dlTraffic);
  cmd.AddValue ("relayUesTraffic", "", relayUesTraffic);
  cmd.AddValue ("inNetTraffic", "", inNetTraffic);
  cmd.AddValue ("enableSensing", "True if sensing is activated", enableSensing);
  cmd.AddValue ("writeTraces", "", writeTraces);



  cmd.Parse (argc, argv);

  if (soldierAttachToSquadVehicle && !forceSoldiersUeOoc)
    {
      NS_FATAL_ERROR (" Soldier attach to Squad vehicle (soldierAttachToSquadVehicle=true) only supported when all soldiers are OOC (forceSoldiersUeOoc=true)");
    }

  Time simTime = timeStartTraffic + durationTraffic + Seconds (1.0); // seconds


  //Setup large enough buffer size to avoid overflow
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));
  //Setup -t-Reordering to 0ms to avoid RLC reordering delay
  Config::SetDefault ("ns3::LteRlcUm::ReorderingTimer", TimeValue (MilliSeconds (0)));

  // create gNBs and in-network UEs, configure positions
  NodeContainer gNbNodes;
  NodeContainer inNetUeNodes;
  NodeContainer relayUeNodes;
  NodeContainer remoteUeNodes;

  MobilityHelper mobility;
  gNbNodes.Create (1);

  Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();
  gNbPositionAlloc->Add (Vector (0.0, 0.0, gNbHeight));



  NodeContainer vehicleUeNodes;
  NodeContainer soldierUeNodes;

  Ptr<UniformRandomVariable> platoonRndVar = CreateObject<UniformRandomVariable> ();
  platoonRndVar->SetAttribute ("Min", DoubleValue (0));
  platoonRndVar->SetAttribute ("Max", DoubleValue (platoonSideLength)); //meters

  Ptr<UniformRandomVariable> squadRndVar = CreateObject<UniformRandomVariable> ();
  squadRndVar->SetAttribute ("Min", DoubleValue (0));
  squadRndVar->SetAttribute ("Max", DoubleValue (squadSideLength)); //meters

  vehicleUeNodes.Create (nSquads);
  soldierUeNodes.Create (nSquads * nSoldiersPerSquad);

  //Position squads
  Ptr<ListPositionAllocator> posAllocVehicles = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> posAllocSoldiers = CreateObject<ListPositionAllocator> ();
  for (uint32_t vehicleIdx = 0; vehicleIdx < nSquads; ++vehicleIdx)
    {
      double posVehicle_x;
      double posVehicle_y;
      if(rangeTest)
        {
          posVehicle_x = inCoverageRadius;
          posVehicle_y = 0;
          posAllocVehicles->Add (Vector (posVehicle_x, posVehicle_y, vehicleUeHeight));
        }
      else
        {
          posVehicle_x = platoonRndVar->GetValue();
          posVehicle_y = platoonRndVar->GetValue();
          if (dropVehiclesCellEdge)
            {
              while ((posVehicle_x - 0.0) * (posVehicle_x - 0.0) + (posVehicle_y - 0.0) * (posVehicle_y - 0.0)  >= inCoverageRadius* inCoverageRadius
                  || (posVehicle_x - 0.0) * (posVehicle_x - 0.0) + (posVehicle_y - 0.0) * (posVehicle_y - 0.0)  < (inCoverageRadius-50)* inCoverageRadius-50)
                {

                  posVehicle_x = platoonRndVar->GetValue();
                  posVehicle_y = platoonRndVar->GetValue();
                }
              posAllocVehicles->Add (Vector (posVehicle_x, posVehicle_y, vehicleUeHeight));
            }
          else
            {
              while (posVehicle_x + squadSideLength > platoonSideLength)
                {
                  posVehicle_x = platoonRndVar->GetValue();
                }
              while (posVehicle_y + squadSideLength > platoonSideLength)
                {
                  posVehicle_y = platoonRndVar->GetValue();
                }

              posAllocVehicles->Add (Vector (posVehicle_x, posVehicle_y, vehicleUeHeight));
            }
        }
      for (uint32_t soldierIdx = 0; soldierIdx < nSoldiersPerSquad; ++soldierIdx)
        {
          double posSoldier_x = posVehicle_x + squadRndVar->GetValue();
          double posSoldier_y = posVehicle_y + squadRndVar->GetValue();
          posAllocSoldiers->Add (Vector (posSoldier_x, posSoldier_y, soldierUeHeight));

        }
    }

  //Install mobility
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (posAllocVehicles);
  mobility.Install (vehicleUeNodes);

  mobility.SetPositionAllocator (posAllocSoldiers);
  mobility.Install (soldierUeNodes);

  if (writeTraces)
    {
      //Generate gnuplot file with the script to generate the topology plot
      std::string topoFilenameWithNoExtension = exampleName + "-topology";
      GenerateTopologyPlotFile (topoFilenameWithNoExtension, gNbNodes, vehicleUeNodes, soldierUeNodes,
                                platoonSideLength, squadSideLength, inCoverageRadius);
    }
  if (forceSoldiersUeOoc)
    {
      //All soldiers UEs are OoC regardless of position
      for (uint32_t ueIdx = 0; ueIdx < soldierUeNodes.GetN (); ueIdx++)
        {
          remoteUeNodes.Add(soldierUeNodes.Get (ueIdx));
        }
    }
  else
    {
      //Determine which soldier UEs are within the in-coverage radius and which one aren't
      for (uint32_t ueIdx = 0; ueIdx < soldierUeNodes.GetN (); ueIdx++)
        {
          double x = soldierUeNodes.Get (ueIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
          double y = soldierUeNodes.Get (ueIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
          //gNodeB is at 0.0,0.0, we let it explicit in case we want to parametrize gNodeB position
          if ( (x - 0.0) * (x - 0.0) + (y - 0.0) * (y - 0.0)  <= inCoverageRadius* inCoverageRadius)
            {
              //Within coverage
              NS_LOG_DEBUG ("In-coverage soldier UE Node ID " <<soldierUeNodes.Get (ueIdx)->GetId ());
              inNetUeNodes.Add(soldierUeNodes.Get (ueIdx));
            }
          else
            {
              //Out-of-coverage
              NS_LOG_DEBUG ("Out-of-coverage soldier UE Node ID " <<soldierUeNodes.Get (ueIdx)->GetId ());
              remoteUeNodes.Add(soldierUeNodes.Get (ueIdx));
            }
        }
    }

  //Determine which vehicle UEs are within the in-coverage radius
  for (uint32_t ueIdx = 0; ueIdx < vehicleUeNodes.GetN (); ueIdx++)
    {
      double x = vehicleUeNodes.Get (ueIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = vehicleUeNodes.Get (ueIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      //gNodeB is at 0.0,0.0, we let it explicit in case we want to parametrize gNodeB position
      if ( (x - 0.0) * (x - 0.0) + (y - 0.0) * (y - 0.0)  <= inCoverageRadius* inCoverageRadius)
        {
          //Within coverage
          NS_LOG_DEBUG ("In-coverage vehicle UE Node ID " <<vehicleUeNodes.Get (ueIdx)->GetId ());
          relayUeNodes.Add(vehicleUeNodes.Get (ueIdx));
        }
      else
        {
          //TODO: What to do with OOC vehicles
        }
    }

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

  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (ueTxPower));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  //gNB bandwidth part manager setup.
  //The current algorithm multiplexes BWPs depending on the associated bearer QCI
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (0)); // The BWP index is 0 because only one BWP will be installed in the eNB

  //Install only in the BWP that will be used for in-network
  uint8_t bwpIdInNet = 0;
  BandwidthPartInfoPtrVector inNetBwp;
  inNetBwp.insert (inNetBwp.end (), band.GetBwpAt (/*CC*/ 0,bwpIdInNet));
  NetDeviceContainer inNetUeNetDev = nrHelper->InstallUeDevice (inNetUeNodes, inNetBwp);
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, inNetBwp);

  //SL UE MAC configuration
  nrHelper->SetUeMacAttribute ("EnableSensing", BooleanValue (enableSensing));
  nrHelper->SetUeMacAttribute ("T1", UintegerValue (2));
  nrHelper->SetUeMacAttribute ("T2", UintegerValue (33));
  nrHelper->SetUeMacAttribute ("ActivePoolId", UintegerValue (0));
  nrHelper->SetUeMacAttribute ("NumSidelinkProcess", UintegerValue (255));
  nrHelper->SetUeMacAttribute ("EnableBlindReTx", BooleanValue (true));
  nrHelper->SetUeMacAttribute ("SlThresPsschRsrp", IntegerValue (-128));

  //SL BWP manager configuration
  uint8_t bwpIdSl = 1;
  nrHelper->SetBwpManagerTypeId (TypeId::LookupByName ("ns3::NrSlBwpManagerUe"));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_MC_PUSH_TO_TALK", UintegerValue (bwpIdSl));


  //Install both BWPs on U2N relays
  NetDeviceContainer relayUeNetDev = nrHelper->InstallUeDevice (relayUeNodes, allBwps);

  //Install both BWPs on SL-only UEs
  //This was needed to avoid errors with bwpId and vector indexes during device installation
  NetDeviceContainer remoteUeNetDev = nrHelper->InstallUeDevice (remoteUeNodes, allBwps );
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
  for (auto it = relayUeNetDev.Begin (); it != relayUeNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
  for (auto it = remoteUeNetDev.Begin (); it != remoteUeNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }


  /* Create NrSlHelper which will configure the UEs protocol stack to be ready to
   * perform Sidelink related procedures
   */
  Ptr<NrSlHelper> nrSlHelper = CreateObject <NrSlHelper> ();
  nrSlHelper->SetEpcHelper (epcHelper);

  //Set the SL error model and AMC
  std::string errorModel = "ns3::NrEesmIrT1";
  nrSlHelper->SetSlErrorModel (errorModel);
  nrSlHelper->SetUeSlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));

  //Set the SL scheduler attributes
  nrSlHelper->SetNrSlSchedulerTypeId (NrSlUeMacSchedulerDefault::GetTypeId ());
  nrSlHelper->SetUeSlSchedulerAttribute ("FixNrSlMcs", BooleanValue (true));
  nrSlHelper->SetUeSlSchedulerAttribute ("InitialNrSlMcs", UintegerValue (14));

  //Configure U2N relay UEs for SL
  std::set<uint8_t> slBwpIdContainerRelay;
  slBwpIdContainerRelay.insert (bwpIdSl);   //Only in the SL BWP for the relay UEs
  nrSlHelper->PrepareUeForSidelink (relayUeNetDev, slBwpIdContainerRelay);

  //Configure SL-only UEs for SL
  nrSlHelper->PrepareUeForSidelink (remoteUeNetDev, slBwpIdContainer);

  /***SL IEs configuration **/

  //SlResourcePoolNr IE
  LteRrcSap::SlResourcePoolNr slResourcePoolNr;
  //get it from pool factory
  Ptr<NrSlCommResourcePoolFactory> ptrFactory = Create<NrSlCommResourcePoolFactory> ();
  //Configure specific parameters of interest:
  std::vector <std::bitset<1> > slBitmap = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  ptrFactory->SetSlTimeResources (slBitmap);
  ptrFactory->SetSlSensingWindow (100); // T0 in ms
  ptrFactory->SetSlSelectionWindow (5);
  ptrFactory->SetSlFreqResourcePscch (10); // PSCCH RBs
  ptrFactory->SetSlSubchannelSize (10);
  ptrFactory->SetSlMaxNumPerReserve (3);
  std::list<uint16_t> resourceReservePeriodList = {0, 100}; // in ms
  ptrFactory->SetSlResourceReservePeriodList (resourceReservePeriodList);
  //Once parameters are configured, we can create the pool
  LteRrcSap::SlResourcePoolNr pool = ptrFactory->CreatePool ();
  slResourcePoolNr = pool;

  //Configure the SlResourcePoolConfigNr IE, which holds a pool and its id
  LteRrcSap::SlResourcePoolConfigNr slresoPoolConfigNr;
  slresoPoolConfigNr.haveSlResourcePoolConfigNr = true;
  //Pool id, ranges from 0 to 15
  uint16_t poolId = 0;
  LteRrcSap::SlResourcePoolIdNr slResourcePoolIdNr;
  slResourcePoolIdNr.id = poolId;
  slresoPoolConfigNr.slResourcePoolId = slResourcePoolIdNr;
  slresoPoolConfigNr.slResourcePool = slResourcePoolNr;

  //Configure the SlBwpPoolConfigCommonNr IE, which holds an array of pools
  LteRrcSap::SlBwpPoolConfigCommonNr slBwpPoolConfigCommonNr;
  //Array for pools, we insert the pool in the array as per its poolId
  slBwpPoolConfigCommonNr.slTxPoolSelectedNormal [slResourcePoolIdNr.id] = slresoPoolConfigNr;

  //Configure the BWP IE
  LteRrcSap::Bwp bwp;
  bwp.numerology = numerologyCc0Bwp1;
  bwp.symbolsPerSlots = 14;
  bwp.rbPerRbg = 1;
  bwp.bandwidth = bandwidthCc0Bpw1/1000/100; // SL configuration requires BW in Multiple of 100 KHz

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

  //Configure the SlFreqConfigCommonNr IE, which holds the array to store
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
  nrSlHelper->InstallNrSlPreConfiguration (remoteUeNetDev, slPreConfigNr);

  //For U2N relay UEs
  //We need to modify some parameters to configure *only* BWP1 on the relay for
  // SL and avoid MAC problems
  LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNrRelay;
  slFreConfigCommonNrRelay.slBwpList [bwpIdSl] = slBwpConfigCommonNr;

  LteRrcSap::SidelinkPreconfigNr slPreConfigNrRelay;
  slPreConfigNrRelay.slPreconfigGeneral = slPreconfigGeneralNr;
  slPreConfigNrRelay.slUeSelectedPreConfig = slUeSelectedPreConfig;
  slPreConfigNrRelay.slPreconfigFreqInfoList [0] = slFreConfigCommonNrRelay;

  nrSlHelper->InstallNrSlPreConfiguration (relayUeNetDev, slPreConfigNrRelay);

  /***END SL IEs configuration **/

  //Set random streams
  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (inNetUeNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (relayUeNetDev, randomStream);
  randomStream += nrSlHelper->AssignStreams (relayUeNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (remoteUeNetDev, randomStream);
  randomStream += nrSlHelper->AssignStreams (remoteUeNetDev, randomStream);


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

  NS_LOG_DEBUG ( "IP configuration: " );
  NS_LOG_DEBUG (" Remote Host: " << remoteHostAddr );

  // Configure in-network only UEs
  internet.Install (inNetUeNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (inNetUeNetDev));
  std::vector<Ipv4Address> inNetIpv4AddressVector (inNetUeNodes.GetN ());

  // Set the default gateway for the in-network UEs
  for (uint32_t j = 0; j < inNetUeNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting (inNetUeNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      inNetIpv4AddressVector [j] = inNetUeNodes.Get (j)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      NS_LOG_DEBUG (" In-network UE: " << inNetIpv4AddressVector [j]);
    }

  //Attach in-network UEs to the closest gNB
  nrHelper->AttachToClosestEnb (inNetUeNetDev, enbNetDev);

  // Configure U2N relay UEs
  internet.Install (relayUeNodes);
  Ipv4InterfaceContainer ueIpIfaceRelays;
  ueIpIfaceRelays = epcHelper->AssignUeIpv4Address (NetDeviceContainer (relayUeNetDev));
  std::vector<Ipv4Address> relaysIpv4AddressVector (relayUeNodes.GetN ());

  for (uint32_t u = 0; u < relayUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting (relayUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      relaysIpv4AddressVector [u] = relayUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      NS_LOG_DEBUG (" Relay UE: " << relaysIpv4AddressVector [u]);
    }

  //Attach U2N relay UEs to the closest gNB
  nrHelper->AttachToClosestEnb (relayUeNetDev, enbNetDev);

  // Configure out-of-network UEs
  internet.Install (remoteUeNodes);
  Ipv4InterfaceContainer ueIpIfaceSl;
  ueIpIfaceSl = epcHelper->AssignUeIpv4Address (NetDeviceContainer (remoteUeNetDev));
  std::vector<Ipv4Address> remotesIpv4AddressVector (remoteUeNodes.GetN ());

  for (uint32_t u = 0; u < remoteUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
          ipv4RoutingHelper.GetStaticRouting (remoteUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      remotesIpv4AddressVector [u] = remoteUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      NS_LOG_DEBUG (" Remote UE: " << remotesIpv4AddressVector [u]);
    }


  /******** Configure ProSe layer in the UEs that will do SL  **********/
  //Create ProSe helper
  Ptr<NrSlProseHelper> nrSlProseHelper = CreateObject <NrSlProseHelper> ();
  nrSlProseHelper->SetEpcHelper (epcHelper);

  // Install ProSe layer and corresponding SAPs in the UEs
  nrSlProseHelper->PrepareUesForProse (relayUeNetDev);
  nrSlProseHelper->PrepareUesForProse (remoteUeNetDev);

  //Configure ProSe Unicast parameters. At the moment it only instruct the MAC
  //layer (and PHY therefore) to monitor packets directed the UE's own Layer 2 ID
  nrSlProseHelper->PrepareUesForUnicast (relayUeNetDev);
  nrSlProseHelper->PrepareUesForUnicast (remoteUeNetDev);

  //Configure the value of timer Timer T5080 (Prose Direct Link Establishment Request Retransmission)
  //to a lower value than the standard (8.0 s) to speed connection in shorter simulation time
  Config::SetDefault ("ns3::NrSlUeProseDirectLink::T5080", TimeValue (Seconds (0.5)));
  /******** END Configure ProSe layer in the UEs that will do SL  **********/


  /******************** L3 U2N Relay configuration ***************************/
  //Provision L3 U2N configuration on the relay UEs

  //-Configure relay service codes
  // Only one relay service per relay UE can currently be provided
  uint32_t relayServiceCode = 5;
  std::set<uint32_t> providedRelaySCs;
  providedRelaySCs.insert (relayServiceCode);

  //-Configure the UL data radio bearer that the relay UE will use for U2N relaying traffic
  Ptr<EpcTft> tftRelay = Create<EpcTft> ();
  EpcTft::PacketFilter pfRelay;
  tftRelay->Add (pfRelay);
  enum EpsBearer::Qci qciRelay;
  qciRelay = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearerRelay (qciRelay);

  //Apply the configuration on the devices acting as relay UEs
  nrSlProseHelper->ConfigureL3UeToNetworkRelay (relayUeNetDev, providedRelaySCs, bearerRelay, tftRelay);

  //Configure direct link connection between remote UEs and relay UEs

  // Random variable to randomize a bit start times of the connection
  //to avoid simulation artifacts of all the TX UEs transmitting at the same time.
  Ptr<UniformRandomVariable> startConnRnd = CreateObject<UniformRandomVariable> ();
  startConnRnd->SetAttribute ("Min", DoubleValue (0));
  startConnRnd->SetAttribute ("Max", DoubleValue (0.15)); //seconds

  NS_LOG_INFO ("Configuring remote UE - relay UE connection..." );
  SidelinkInfo remoteUeSlInfo;
  remoteUeSlInfo.m_castType = SidelinkInfo::CastType::Unicast;
  remoteUeSlInfo.m_dynamic = true;
  remoteUeSlInfo.m_harqEnabled = false;
  remoteUeSlInfo.m_priority = 0;
  remoteUeSlInfo.m_rri = Seconds (0);

  SidelinkInfo relayUeSlInfo;
  relayUeSlInfo.m_castType = SidelinkInfo::CastType::Unicast;
  relayUeSlInfo.m_dynamic = true;
  relayUeSlInfo.m_harqEnabled = false;
  relayUeSlInfo.m_priority = 0;
  relayUeSlInfo.m_rri = Seconds (0);

  if (soldierAttachToSquadVehicle)
    {
      //Option 1: Each OOC soldier connects to the vehicle in their squad if in coverage
      //This logic works only if all soldiers are OOC (forceSoldiersUeOoc)
      //If some soldiers are in coverage, we need to keep track of them and more advanced logic is needed here
      for (uint32_t j = 0; j < relayUeNodes.GetN (); ++j)
        {
          for (uint32_t i = 0; i < nSoldiersPerSquad; ++i)
            {
              uint16_t rmIdx = j*nSoldiersPerSquad +i;
              nrSlProseHelper->EstablishL3UeToNetworkRelayConnection (startRelayConnTime + Seconds (startConnRnd->GetValue ()),
                                                                      remoteUeNetDev.Get (rmIdx), remotesIpv4AddressVector [rmIdx], remoteUeSlInfo, // Remote UE
                                                                      relayUeNetDev.Get (j), relaysIpv4AddressVector [j],  relayUeSlInfo, // Relay UE
                                                                      relayServiceCode);

              NS_LOG_DEBUG ("Remote UE nodeId " << remoteUeNetDev.Get (rmIdx)->GetNode()->GetId()
                            << " Relay UE nodeId " << relayUeNetDev.Get (j)->GetNode()->GetId());
            }
        }

    }
  else
    {
      //Option 2: Each OOC soldier connects to the closest vehicle in coverage
      for (uint32_t i = 0; i < remoteUeNodes.GetN (); ++i)
        {
          double closestRelayDistance = std::numeric_limits <double>::max ();
          uint32_t closestRelayIdx = std::numeric_limits <uint32_t>::max ();
          for (uint32_t j = 0; j < relayUeNodes.GetN (); ++j)
            {

              double rm_x = remoteUeNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().x;
              double rm_y = remoteUeNodes.Get (i)->GetObject<MobilityModel> ()->GetPosition ().y;
              double ry_x = relayUeNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ().x;
              double ry_y = relayUeNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ().y;
              double distance = std::sqrt (std::pow (rm_x - ry_x , 2) + std::pow (rm_y - ry_y, 2));
              if (distance < closestRelayDistance )
                {
                  closestRelayDistance = distance;
                  closestRelayIdx = j;
                }
            }
          if (closestRelayIdx != std::numeric_limits <uint32_t>::max ())
            {
              nrSlProseHelper->EstablishL3UeToNetworkRelayConnection (startRelayConnTime + Seconds (startConnRnd->GetValue ()),
                                                                      remoteUeNetDev.Get (i), remotesIpv4AddressVector [i], remoteUeSlInfo, // Remote UE
                                                                      relayUeNetDev.Get (closestRelayIdx), relaysIpv4AddressVector [closestRelayIdx],  relayUeSlInfo, // Relay UE
                                                                      relayServiceCode);

              NS_LOG_DEBUG ("Remote UE nodeId " << remoteUeNetDev.Get (i)->GetNode()->GetId()
                            << " Relay UE nodeId " << relayUeNetDev.Get (closestRelayIdx)->GetNode()->GetId());
            }
          else
            {
              //Error
            }
        }
    }
  /******************** END L3 U2N Relay configuration ***********************/


  /************************ Application configuration ************************/
  /* Client app: OnOff application configured to generate CBR traffic when in
   *             On periods. Installed in the Remote Host (DL traffic) and in
   *             the UEs (UL traffic)
   * Server app: PacketSink application to consume the received traffic.
   *             Installed in all end nodes (Remote Host and UEs)
   */
  uint16_t dlPort = 1000;
  uint16_t ulPort = dlPort + 1000;
  ApplicationContainer clientApps, serverApps;
  // Random variable to randomize a bit start times of the client applications
  //to avoid simulation artifacts of all the TX UEs transmitting at the same time.
  Ptr<UniformRandomVariable> startTimeRnd = CreateObject<UniformRandomVariable> ();
  startTimeRnd->SetAttribute ("Min", DoubleValue (0));
  startTimeRnd->SetAttribute ("Max", DoubleValue (2.0)); //seconds
  std::string dataRateString  = std::to_string (onOffDataRate) + "kb/s";

  std::string onDistStr = "ns3::ConstantRandomVariable[Constant=3.0]";
  std::string offDistStr = "ns3::ConstantRandomVariable[Constant=2.0]";

  Config::SetDefault ("ns3::OnOffApplication::EnableSeqTsSizeHeader", BooleanValue (true));
  Config::SetDefault ("ns3::PacketSink::EnableSeqTsSizeHeader", BooleanValue (true));

  //InNetwork UEs
  NS_LOG_DEBUG ("InNetwork UEs traffic flows: ");
  if (inNetTraffic)
    {
      for (uint32_t inIdx = 0; inIdx < inNetUeNodes.GetN (); ++inIdx)
        {

          if (dlTraffic)
            {
              //DL applications configuration
              //-Client on Remote Host
              OnOffHelper dlAppOnOffHelper ("ns3::UdpSocketFactory",
                                            InetSocketAddress (inNetIpv4AddressVector [inIdx], dlPort)); //Towards Remote UE rmIdx IP
              dlAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
              dlAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
              dlAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
              ApplicationContainer dlClientApp = dlAppOnOffHelper.Install (remoteHost); // Installed in Remote Host
              Time dlAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
              dlClientApp.Start (dlAppStartTime);
              clientApps.Add (dlClientApp);
              NS_LOG_DEBUG (" DL: " << remoteHostAddr << " -> " << inNetIpv4AddressVector [inIdx] <<
                            " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s");

              //-Server on UE
              PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
              ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (inNetUeNodes.Get (inIdx));
              serverApps.Add (dlSeverApp);

              //DL bearer configuration
              Ptr<EpcTft> tftDl = Create<EpcTft> ();
              EpcTft::PacketFilter pfDl;
              pfDl.localPortStart = dlPort;
              pfDl.localPortEnd = dlPort;
              ++dlPort;
              tftDl->Add (pfDl);
              enum EpsBearer::Qci qDl;
              qDl = EpsBearer::GBR_CONV_VOICE;
              EpsBearer bearerDl (qDl);
              nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerDl, tftDl);
            }

          if (ulTraffic)
            {
              //UL application configuration
              //-Client on UE
              OnOffHelper ulAppOnOffHelper ("ns3::UdpSocketFactory",
                                            InetSocketAddress (remoteHostAddr, ulPort)); //Towards Remote Host
              ulAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
              ulAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
              ulAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
              ApplicationContainer ulClientApp = ulAppOnOffHelper.Install (inNetUeNodes.Get (inIdx)); // Installed in Remote UE rmIdx
              Time ulAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
              ulClientApp.Start (ulAppStartTime);
              clientApps.Add (ulClientApp);
              NS_LOG_DEBUG (" UL: " << inNetIpv4AddressVector [inIdx] << " -> " << remoteHostAddr <<
                            " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s");

              //-Server on Remtoe Host
              PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
              ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
              serverApps.Add (ulSeverApp);

              //UL bearer configuration
              Ptr<EpcTft> tftUl = Create<EpcTft> ();
              EpcTft::PacketFilter pfUl;
              pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
              pfUl.remotePortStart = ulPort;
              pfUl.remotePortEnd = ulPort;
              ++ulPort;
              tftUl->Add (pfUl);
              enum EpsBearer::Qci qUl;
              qUl = EpsBearer::GBR_CONV_VOICE;
              EpsBearer bearerUl (qUl);
              nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerUl, tftUl);
            }
        }
    }
  //Remote UEs
  NS_LOG_DEBUG ("Remote UEs traffic flows: ");
  for (uint32_t rmIdx = 0; rmIdx < remoteUeNodes.GetN (); ++rmIdx)
    {
      if (dlTraffic)
        {
          //DL applications configuration
          //-Client on Remote Host
          OnOffHelper dlAppOnOffHelper ("ns3::UdpSocketFactory",
                                        InetSocketAddress (remotesIpv4AddressVector [rmIdx], dlPort)); //Towards Remote UE rmIdx IP
          dlAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
          dlAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
          dlAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
          ApplicationContainer dlClientApp = dlAppOnOffHelper.Install (remoteHost); // Installed in Remote Host
          Time dlAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
          dlClientApp.Start (dlAppStartTime);
          clientApps.Add (dlClientApp);
          NS_LOG_DEBUG (" DL: " << remoteHostAddr << " -> " << remotesIpv4AddressVector [rmIdx] <<
                        " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s");


          //-Server on UE
          PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (remoteUeNodes.Get (rmIdx));
          serverApps.Add (dlSeverApp);

          //DL bearer configuration
          Ptr<EpcTft> tftDl = Create<EpcTft> ();
          EpcTft::PacketFilter pfDl;
          pfDl.localPortStart = dlPort;
          pfDl.localPortEnd = dlPort;
          ++dlPort;
          tftDl->Add (pfDl);
          enum EpsBearer::Qci qDl;
          qDl = EpsBearer::GBR_CONV_VOICE;
          EpsBearer bearerDl (qDl);
          nrHelper->ActivateDedicatedEpsBearer (remoteUeNetDev.Get (rmIdx), bearerDl, tftDl);
        }
      if (ulTraffic)
        {
          //UL application configuration
          //-Client on UE
          OnOffHelper ulAppOnOffHelper ("ns3::UdpSocketFactory",
                                        InetSocketAddress (remoteHostAddr, ulPort)); //Towards Remote Host
          ulAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
          ulAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
          ulAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
          ApplicationContainer ulClientApp = ulAppOnOffHelper.Install (remoteUeNodes.Get (rmIdx)); // Installed in Remote UE rmIdx
          Time ulAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
          ulClientApp.Start (ulAppStartTime);
          clientApps.Add (ulClientApp);
          NS_LOG_DEBUG (" UL: " << remotesIpv4AddressVector [rmIdx] << " -> " << remoteHostAddr <<
                        " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s");

          //-Server on Remote Host
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
          serverApps.Add (ulSeverApp);
        }
      //UL bearer configuration
      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
      pfUl.remotePortStart = ulPort;
      pfUl.remotePortEnd = ulPort;
      ++ulPort;
      tftUl->Add (pfUl);
      enum EpsBearer::Qci qUl;
      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (remoteUeNetDev.Get (rmIdx), bearerUl, tftUl);

    }

  //Relay UEs
  if (relayUesTraffic)
    {
      NS_LOG_DEBUG ("Relay UEs traffic flows: ");
      for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
        {
          if (dlTraffic)
            {
              //DL applications configuration
              //-Client on Remote Host
              OnOffHelper dlAppOnOffHelper ("ns3::UdpSocketFactory",
                                            InetSocketAddress (relaysIpv4AddressVector [ryIdx], dlPort)); //Towards Remote UE rmIdx IP
              dlAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
              dlAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
              dlAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
              ApplicationContainer dlClientApp = dlAppOnOffHelper.Install (remoteHost); // Installed in Remote Host
              Time dlAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
              dlClientApp.Start (dlAppStartTime);
              clientApps.Add (dlClientApp);
              NS_LOG_DEBUG (" DL: " << remoteHostAddr << " -> " << relaysIpv4AddressVector [ryIdx] <<
                            " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s");

              //-Server in UE
              PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
              ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (relayUeNodes.Get (ryIdx));
              serverApps.Add (dlSeverApp);

              //DL bearer configuration
              Ptr<EpcTft> tftDl = Create<EpcTft> ();
              EpcTft::PacketFilter pfDl;
              pfDl.localPortStart = dlPort;
              pfDl.localPortEnd = dlPort;
              ++dlPort;
              tftDl->Add (pfDl);
              enum EpsBearer::Qci qDl;
              qDl = EpsBearer::GBR_CONV_VOICE;
              EpsBearer bearerDl (qDl);
              nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerDl, tftDl);
            }
          if (ulTraffic)
            {
              //UL application configuration
              OnOffHelper ulAppOnOffHelper ("ns3::UdpSocketFactory",
                                            InetSocketAddress (remoteHostAddr, ulPort)); //Towards Remote Host
              ulAppOnOffHelper.SetConstantRate (DataRate (dataRateString), onOffPacketSize);
              ulAppOnOffHelper.SetAttribute ("OnTime", StringValue (onDistStr));
              ulAppOnOffHelper.SetAttribute ("OffTime", StringValue (offDistStr));
              ApplicationContainer ulClientApp = ulAppOnOffHelper.Install (relayUeNodes.Get (ryIdx)); // Installed in Remote UE rmIdx
              Time ulAppStartTime = timeStartTraffic + Seconds (startTimeRnd->GetValue ());
              ulClientApp.Start (ulAppStartTime);
              clientApps.Add (ulClientApp);
              NS_LOG_DEBUG ( " UL: " << relaysIpv4AddressVector [ryIdx] << " -> " << remoteHostAddr  <<
                             " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" );

              PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
              ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
              serverApps.Add (ulSeverApp);

              //UL bearer configuration
              Ptr<EpcTft> tftUl = Create<EpcTft> ();
              EpcTft::PacketFilter pfUl;
              pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
              pfUl.remotePortStart = ulPort;
              pfUl.remotePortEnd = ulPort;
              ++ulPort;
              tftUl->Add (pfUl);
              enum EpsBearer::Qci qUl;
              qUl = EpsBearer::GBR_CONV_VOICE;
              EpsBearer bearerUl (qUl);
              nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerUl, tftUl);
            }
        }
    }

  clientApps.Stop (timeStartTraffic + durationTraffic);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (timeStartTraffic + durationTraffic);
  /******************** End Application configuration ************************/

  /************ SL traces database setup *************************************/
  UeMacPscchTxOutputStats pscchStats;
  UeMacPsschTxOutputStats psschStats;
  UePhyPscchRxOutputStats pscchPhyStats;
  UePhyPsschRxOutputStats psschPhyStats;
  UeRlcRxOutputStats ueRlcRxStats;
  UeToUePktTxRxOutputStats pktStats;
  SQLiteOutput db (exampleName + "-SlTraces.db");

  if (writeTraces)
    {
      pscchStats.SetDb (&db, "pscchTxUeMac");
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPscchScheduling",
                                     MakeBoundCallback (&NotifySlPscchScheduling, &pscchStats));

      psschStats.SetDb (&db, "psschTxUeMac");
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPsschScheduling",
                                     MakeBoundCallback (&NotifySlPsschScheduling, &psschStats));

      pscchPhyStats.SetDb (&db, "pscchRxUePhy");
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPscchTraceUe",
                                     MakeBoundCallback (&NotifySlPscchRx, &pscchPhyStats));

      psschPhyStats.SetDb (&db, "psschRxUePhy");
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPsschTraceUe",
                                     MakeBoundCallback (&NotifySlPsschRx, &psschPhyStats));
      ueRlcRxStats.SetDb (&db, "rlcRx");
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/RxRlcPduWithTxRnti",
                                     MakeBoundCallback (&NotifySlRlcPduRx, &ueRlcRxStats));

      pktStats.SetDb (&db, "pktTxRx");

      for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          clientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceDb, &pktStats, clientApps.Get (ac)->GetNode (), localAddrs));
        }

      for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          serverApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceDb, &pktStats, serverApps.Get (ac)->GetNode (), localAddrs));
        }
      /************ END SL traces database setup *************************************/
    }

  /******************* Application packet delay tracing ********************************/
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> PacketTraceForDelayStream;
  if (writeTraces)
    {
      PacketTraceForDelayStream = ascii.CreateFileStream ("NrSlAppRxPacketDelayTrace.txt");
      *PacketTraceForDelayStream->GetStream () << "time(s)\trxNodeId\tsrcIp\tdstIp\tseqNum\tdelay(ms)" << std::endl;
    }
  else
    {
      PacketTraceForDelayStream = nullptr;
    }
  for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      clientApps.Get (ac)->TraceConnectWithoutContext ("TxWithSeqTsSize", MakeBoundCallback (&TxPacketTrace, localAddrs));
    }
  for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      serverApps.Get (ac)->TraceConnectWithoutContext ("RxWithSeqTsSize", MakeBoundCallback (&RxPacketTrace, PacketTraceForDelayStream, serverApps.Get (ac)->GetNode (), localAddrs));
    }
  /******************* END Application packet delay tracing ****************************/


  if (writeTraces)
    {
      /******************* PC5-S messages tracing ********************************/
      Ptr<OutputStreamWrapper> Pc5SignallingPacketTraceStream = ascii.CreateFileStream ("NrSlPc5SignallingPacketTrace.txt");
      *Pc5SignallingPacketTraceStream->GetStream () << "time(s)\tnodeId\tTX/RX\tsrcL2Id\tdstL2Id\tmsgType" << std::endl;
      for (uint32_t i = 0; i < remoteUeNetDev.GetN (); ++i)
        {
          Ptr<NrSlUeProse> prose = remoteUeNetDev.Get (i)->GetObject<NrUeNetDevice> ()->GetSlUeService ()->GetObject <NrSlUeProse> ();
          prose->TraceConnectWithoutContext ("PC5SignallingPacketTrace",
                                             MakeBoundCallback (&TraceSinkPC5SignallingPacketTrace,
                                                                Pc5SignallingPacketTraceStream,
                                                                remoteUeNetDev.Get (i)->GetNode ()));
        }
      for (uint32_t i = 0; i < relayUeNetDev.GetN (); ++i)
        {
          Ptr<NrSlUeProse> prose = relayUeNetDev.Get (i)->GetObject<NrUeNetDevice> ()->GetSlUeService ()->GetObject <NrSlUeProse> ();
          prose->TraceConnectWithoutContext ("PC5SignallingPacketTrace",
                                             MakeBoundCallback (&TraceSinkPC5SignallingPacketTrace,
                                                                Pc5SignallingPacketTraceStream,
                                                                relayUeNetDev.Get (i)->GetNode ()));
        }
      /******************* END PC5-S messages tracing **************************/
      /******************* Received messages by the relay tracing **************/

      Ptr<OutputStreamWrapper> RelayNasRxPacketTraceStream = ascii.CreateFileStream ("NrSlRelayNasRxPacketTrace.txt");
      *RelayNasRxPacketTraceStream->GetStream () << "time(s)\tnodeId\tnodeIp\tsrcIp\tdstIp\tsrcLink\tdstLink" << std::endl;
      for (uint32_t i = 0; i < relayUeNetDev.GetN (); ++i)
        {
          Ptr<EpcUeNas> epcUeNas = relayUeNetDev.Get (i)->GetObject<NrUeNetDevice> ()->GetNas ();

          epcUeNas->TraceConnectWithoutContext ("NrSlRelayRxPacketTrace",
                                                MakeBoundCallback (&TraceSinkRelayNasRxPacketTrace,
                                                                   RelayNasRxPacketTraceStream,
                                                                   relayUeNetDev.Get (i)->GetNode ()));
        }
    }
  /*************** END Received messages by the relay tracing **************/

  //Configure FlowMonitor to get traffic flow statistics
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (inNetUeNodes);
  endpointNodes.Add (remoteUeNodes);
  endpointNodes.Add (relayUeNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  //Run simulation
  Simulator::Stop (simTime);
  Simulator::Run ();

  //SL database dump
  if (writeTraces)
    {
      pscchStats.EmptyCache ();
      psschStats.EmptyCache ();
      pscchPhyStats.EmptyCache ();
      psschPhyStats.EmptyCache ();
      ueRlcRxStats.EmptyCache ();
      pktStats.EmptyCache ();
    }

  NS_LOG_DEBUG ("Simulation done!" << "Traffic flows statistics: " );
  //Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

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

      appDuration = simTime.GetSeconds () - timeStartTraffic.GetSeconds (); //Some inaccuracy is expected due to randomization of start time.

      NS_LOG_DEBUG ( "  Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") " << protoStream.str ());
      NS_LOG_DEBUG ( "    Tx Packets: " << i->second.txPackets);
      NS_LOG_DEBUG ( "    Tx Bytes:   " << i->second.txBytes);
      NS_LOG_DEBUG ( "    TxOffered:  " << i->second.txBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps");
      NS_LOG_DEBUG ( "    Rx Packets: " << i->second.rxPackets);
      NS_LOG_DEBUG ( "    Rx Bytes:   " << i->second.rxBytes);
      if (i->second.rxPackets > 0)
        {
          NS_LOG_DEBUG ( "    Throughput: " << i->second.rxBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps");
          NS_LOG_DEBUG ( "    Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms");
          NS_LOG_DEBUG ( "    Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms");
        }
      else
        {
          NS_LOG_DEBUG ( "    Throughput:  0 Mbps");
          NS_LOG_DEBUG ( "    Mean delay:  0 ms");
          NS_LOG_DEBUG ( "    Mean jitter: 0 ms");
        }
    }



  NS_LOG_DEBUG ( "Number of packets relayed by the L3 UE-to-Network relays:");
  NS_LOG_DEBUG ( "relayIp      srcIp->dstIp      srcLink->dstLink\t\tnPackets");
  for (auto it = g_relayNasPacketCounter.begin (); it != g_relayNasPacketCounter.end (); ++it)
    {
      NS_LOG_DEBUG (it->first << "\t\t" << it->second);
    }

  NS_LOG_DEBUG ("*********************************************************" );
  NS_LOG_DEBUG ("Ratio of OOC soldiers " <<  ((double)remoteUeNodes.GetN () / inNetUeNodes.GetN ()) );

  NS_LOG_DEBUG ("Total number of transmitted packets : " << g_nTxPackets << "\n"
                << "Total number of received packets : " << g_nRxPackets << "\n"
                << "Packets lost: " << g_packetsForDelayCalc.size () << " " <<  g_nTxPackets - g_nRxPackets );
  double delaySum = 0;
  for (auto it = g_delays.begin () ; it != g_delays.end (); it ++ )
    {
      delaySum += *it;
    }
  NS_LOG_DEBUG ("Packet delivery ratio " <<  ((double)g_nRxPackets / g_nTxPackets));

  NS_LOG_DEBUG ("Average packet delay = " << delaySum / g_delays.size () << " ms");

  //Standard output:
  std::cout << "PDR\tavgDelay(ms)" << std::endl;
  std::cout << ((double)g_nRxPackets / g_nTxPackets) << "\t" << delaySum / g_delays.size () << std::endl;

  //Print to the file that will be use to calculate statistics
  std::ofstream outFile;
  std::string filename = exampleName  + "-stats.txt";
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }
  outFile.setf (std::ios_base::fixed);

  outFile << "PDR\tavgDelay(ms)" << std::endl;
  outFile << ((double)g_nRxPackets / g_nTxPackets) << "\t" << delaySum / g_delays.size () << std::endl;

  outFile.close ();

  Simulator::Destroy ();
  return 0;
}


