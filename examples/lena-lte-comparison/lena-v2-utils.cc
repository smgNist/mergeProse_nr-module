/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "lena-v2-utils.h"

#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"
#include "rb-output-stats.h"

#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("LenaV2Utils");

namespace ns3 {

void
LenaV2Utils::ReportSinrNr (SinrOutputStats *stats, uint16_t cellId, uint16_t rnti,
                           double power, double avgSinr, uint16_t bwpId)
{
  stats->SaveSinr (cellId, rnti, power, avgSinr, bwpId);
}

void
LenaV2Utils::ReportPowerNr (PowerOutputStats *stats, const SfnSf & sfnSf,
                            Ptr<const SpectrumValue> txPsd, const Time &t, uint16_t rnti, uint64_t imsi,
                            uint16_t bwpId, uint16_t cellId)
{
  stats->SavePower (sfnSf, txPsd, t, rnti, imsi, bwpId, cellId);
}

void
LenaV2Utils::ReportSlotStatsNr (SlotOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
                                uint32_t usedReg, uint32_t usedSym,
                                uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
                                uint16_t cellId)
{
  stats->SaveSlotStats (sfnSf, scheduledUe, usedReg, usedSym,
                        availableRb, availableSym, bwpId, cellId);
}

void
LenaV2Utils::ReportRbStatsNr (RbOutputStats *stats, const SfnSf &sfnSf, uint8_t sym,
                              const std::vector<int> &rbUsed, uint16_t bwpId,
                              uint16_t cellId)
{
  stats->SaveRbStats (sfnSf, sym, rbUsed, bwpId, cellId);
}

void
LenaV2Utils::ReportGnbRxDataNr (PowerOutputStats *gnbRxDataStats, const SfnSf &sfnSf,
                                Ptr<const SpectrumValue> rxPsd, const Time &t, uint16_t bwpId,
                                uint16_t cellId)
{
  gnbRxDataStats->SavePower (sfnSf, rxPsd, t, 0, 0, bwpId, cellId);
}

void
LenaV2Utils::ConfigureBwpTo (BandwidthPartInfoPtr & bwp, double centerFreq, double bwpBw)
{
  bwp->m_centralFrequency = centerFreq;
  bwp->m_higherFrequency = centerFreq + (bwpBw / 2);
  bwp->m_lowerFrequency = centerFreq - (bwpBw / 2);
  bwp->m_channelBandwidth = bwpBw;
}

void
LenaV2Utils::SetLenaV2SimulatorParameters (const HexagonalGridScenarioHelper &gridScenario,
                                           const std::string &scenario,
                                           const std::string &radioNetwork,
                                           std::string errorModel,
                                           const std::string &operationMode,
                                           const std::string &direction,
                                           uint16_t numerology,
                                           const std::string &pattern,
                                           const NodeContainer &gnbSector1Container,
                                           const NodeContainer &gnbSector2Container,
                                           const NodeContainer &gnbSector3Container,
                                           const NodeContainer &ueSector1Container,
                                           const NodeContainer &ueSector2Container,
                                           const NodeContainer &ueSector3Container,
                                           const Ptr<PointToPointEpcHelper> &baseEpcHelper,
                                           Ptr<NrHelper> &nrHelper,
                                           NetDeviceContainer &gnbSector1NetDev,
                                           NetDeviceContainer &gnbSector2NetDev,
                                           NetDeviceContainer &gnbSector3NetDev,
                                           NetDeviceContainer &ueSector1NetDev,
                                           NetDeviceContainer &ueSector2NetDev,
                                           NetDeviceContainer &ueSector3NetDev,
                                           bool calibration,
                                           SinrOutputStats *sinrStats,
                                           PowerOutputStats *ueTxPowerStats,
                                           PowerOutputStats *gnbRxPowerStats,
                                           SlotOutputStats *slotStats, RbOutputStats *rbStats,
                                           const std::string &scheduler,
                                           uint32_t bandwidthMHz, uint32_t freqScenario,
                                           double downtiltAngle)
{
  /*
   * Create the radio network related parameters
   */
  uint8_t numScPerRb = 1;  //!< The reference signal density is different in LTE and in NR
  double rbOverhead = 0.1;
  uint32_t harqProcesses = 20;
  uint32_t n1Delay = 2;
  uint32_t n2Delay = 2;
  if (radioNetwork == "LTE")
    {
      rbOverhead = 0.1;
      harqProcesses = 8;
      n1Delay = 4;
      n2Delay = 4;
      if (errorModel == "")
        {
          errorModel = "ns3::LenaErrorModel";
        }
      else if (errorModel != "ns3::NrLteMiErrorModel" && errorModel != "ns3::LenaErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for LTE");
        }
    }
  else if (radioNetwork == "NR")
    {
      rbOverhead = 0.04;
      harqProcesses = 20;
      if (errorModel == "")
        {
          errorModel = "ns3::NrEesmCcT2";
        }
      else if (errorModel == "ns3::NrLteMiErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for NR");
        }
    }
  else
    {
      NS_ABORT_MSG ("Unrecognized radio network technology");
    }

  /*
   * Setup the NR module. We create the various helpers needed for the
   * NR simulation:
   * - IdealBeamformingHelper, which takes care of the beamforming part
   * - NrHelper, which takes care of creating and connecting the various
   * part of the NR stack
   */

  nrHelper = CreateObject<NrHelper> ();

  Ptr<IdealBeamformingHelper> idealBeamformingHelper;

  // in LTE non-calibration we want to use predefined beams that we set directly
  // through beam manager. Hence, we do not need any ideal algorithm.
  // For other cases, we need it (and the beam will be overwritten)
  if (radioNetwork == "NR" || calibration)
    {
      idealBeamformingHelper = CreateObject<IdealBeamformingHelper> ();
      nrHelper->SetBeamformingHelper (idealBeamformingHelper);
    }

  Ptr<NrPointToPointEpcHelper> epcHelper = DynamicCast<NrPointToPointEpcHelper> (baseEpcHelper);
  nrHelper->SetEpcHelper (epcHelper);

  double txPowerBs = 0.0;

  BandwidthPartInfo::Scenario scene;
  if (scenario == "UMi")
    {
      txPowerBs = 30;
      scene =  BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (scenario == "UMa")
    {
      txPowerBs = 43;
      scene = BandwidthPartInfo::UMa_LoS;
    }
  else if (scenario == "RMa")
    {
      txPowerBs = 43;
      scene = BandwidthPartInfo::RMa_LoS;
    }
  else
    {
      NS_ABORT_MSG ("Unsupported scenario " << scenario << ". Supported values: UMi, UMa, RMa");
    }

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (100)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));

  // Disable shadowing in calibration, and enable it in non-calibration mode
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (!calibration));

  // Noise figure for the UE
  nrHelper->SetUePhyAttribute ("NoiseFigure", DoubleValue (9.0));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel (errorModel);
  nrHelper->SetDlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));

  /*
   * Adjust the average number of Reference symbols per RB only for LTE case,
   * which is larger than in NR. We assume a value of 4 (could be 3 too).
   */
  nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (numScPerRb));
  nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (1));  //FIXME: Might change in LTE

  nrHelper->SetGnbPhyAttribute ("RbOverhead", DoubleValue (rbOverhead));
  nrHelper->SetGnbPhyAttribute ("N2Delay", UintegerValue (n2Delay));
  nrHelper->SetGnbPhyAttribute ("N1Delay", UintegerValue (n1Delay));

  nrHelper->SetUeMacAttribute ("NumHarqProcess", UintegerValue (harqProcesses));
  nrHelper->SetGnbMacAttribute ("NumHarqProcess", UintegerValue (harqProcesses));

  /*
   * Create the necessary operation bands.
   *
   * In the 0 frequency scenario, each sector operates, in a separate band,
   * while for scenario 1 all the sectors are in the same band. Please note that
   * a single BWP in FDD is half the size of the corresponding TDD BWP, and the
   * parameter bandwidthMHz refers to the size of the FDD BWP.
   *
   * Scenario 0:  sectors NON_OVERLAPPING in frequency
   * 
   * FDD scenario 0:
   *
   * |--------Band0--------|--------Band1--------|--------Band2--------|
   * |---------CC0---------|---------CC1---------|---------CC2---------|
   * |---BWP0---|---BWP1---|---BWP2---|---BWP3---|---BWP4---|---BWP5---|
   *
   *   Sector i will go in Bandi
   *   DL in the first BWP, UL in the second BWP
   *
   * TDD scenario 0:
   *
   * |--------Band0--------|--------Band1--------|--------Band2--------|
   * |---------CC0---------|---------CC2---------|---------CC2---------|
   * |---------BWP0--------|---------BWP1--------|---------BWP2--------|
   *
   *   Sector i will go in BWPi
   *
   *
   * Scenario 1:  sectors in OVERLAPPING bands
   *
   * Note that this configuration has 1/3 the total bandwidth of the
   * NON_OVERLAPPING configuration.
   *
   * FDD scenario 1:
   *
   * |--------Band0--------|
   * |---------CC0---------|
   * |---BWP0---|---BWP1---|
   *
   *   Sector i will go in BWPi
   *
   * TDD scenario 1:
   *
   * |--------Band0--------|
   * |---------CC0---------|
   * |---------BWP0--------|
   *
   * This is tightly coupled with what happens in lena-v1-utils.cc
   *
   */
  // \todo: set band 0 start frequency from the command line
  const double band0Start = 2110e6;
  double bandwidthBwp = bandwidthMHz * 1e6;
  
  OperationBandInfo band0, band1, band2;
  band0.m_bandId = 0;
  band1.m_bandId = 1;
  band2.m_bandId = 2;

  if ((freqScenario == 0) // NON_OVERLAPPING
      && (operationMode == "FDD"))
    {
      // FDD uses two BWPs per CC, one CC per band
      uint8_t numBwp = 2;
      double bandwidthCc = numBwp * bandwidthBwp;
      uint8_t numCcPerBand = 1;
      double bandwidthBand = numCcPerBand * bandwidthCc;
      double bandCenter = band0Start + bandwidthBand / 2.0;
      
      NS_LOG_LOGIC ("NON_OVERLAPPING, FDD: "
                    << bandwidthBand << ":" << bandwidthCc << ":"
                    << bandwidthBwp << ", "
                    << (int)numCcPerBand << ", " << (int)numBwp);
      
      NS_LOG_LOGIC ("bandConf0: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf0 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf0.m_numBwp = numBwp;
      bandCenter += bandwidthBand;
      
      NS_LOG_LOGIC ("bandConf1: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf1 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf1.m_numBwp = numBwp;
      bandCenter += bandwidthBand;
      
      NS_LOG_LOGIC ("bandConf2: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf2 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf2.m_numBwp = numBwp;
      
      // Create, then configure
      CcBwpCreator ccBwpCreator;
      band0 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf0); 
      band0.m_bandId = 0;
          
      band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
      band1.m_bandId = 1;
      
      band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);
      band2.m_bandId = 2;
      
      bandCenter = band0Start + bandwidthBwp / 2.0;
      
      NS_LOG_LOGIC ("band0[0][0]: " << bandCenter << " " << bandwidthBwp);;
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band0[0][1]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[1], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band1[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      NS_LOG_LOGIC ("band1[0][1]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band1.m_cc[0]->m_bwp[1], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band2[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band2.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      NS_LOG_LOGIC ("band2[0][1]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band2.m_cc[0]->m_bwp[1], bandCenter, bandwidthBwp);
      
      std::cout << "BWP Configuration for NON_OVERLAPPING case, mode "
                << operationMode << "\n"
                << band0 << band1 << band2;
    }
  
  else if ( (freqScenario == 0) // NON_OVERLAPPING
            && (operationMode == "TDD") )
    {
      // Use double with BWP, to match total bandwidth for FDD in UL and DL
      bandwidthBwp *= 2;
      
      uint8_t numBwp = 1;
      double bandwidthCc = numBwp * bandwidthBwp;
      uint8_t numCcPerBand = 1;
      double bandwidthBand = numCcPerBand * bandwidthCc;
      double bandCenter = band0Start + bandwidthBand / 2.0;
      
      NS_LOG_LOGIC ("NON_OVERLAPPING, TDD: "
                    << bandwidthBand << ":" << bandwidthCc << ":"
                    << bandwidthBwp << ", "
                    << (int)numCcPerBand << ", " << (int)numBwp);
      
      NS_LOG_LOGIC ("bandConf0: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf0 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf0.m_numBwp = numBwp;
      bandCenter += bandwidthBand;

      NS_LOG_LOGIC ("bandConf1: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf1 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf1.m_numBwp = numBwp;
      bandCenter += bandwidthBand;
      
      NS_LOG_LOGIC ("bandConf2: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf2 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf2.m_numBwp = numBwp;
      
      // Create, then configure
      CcBwpCreator ccBwpCreator;
      band0 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf0);
      band0.m_bandId = 0;
          
      band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
      band1.m_bandId = 1;
      
      band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);
      band2.m_bandId = 2;
      
      bandCenter = band0Start + bandwidthBwp / 2.0;
      
      NS_LOG_LOGIC ("band0[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band1[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band2[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band2.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);

      std::cout << "BWP Configuration for NON_OVERLAPPING case, mode "
                << operationMode << "\n"
                << band0 << band1 << band2;
    }

  else if ( (freqScenario == 1) // OVERLAPPING
            && (operationMode == "FDD") )
    {
      // FDD uses two BWPs per CC, one CC per band
      uint8_t numBwp = 2;
      double bandwidthCc = numBwp * bandwidthBwp;
      uint8_t numCcPerBand = 1;
      double bandwidthBand = numCcPerBand * bandwidthCc;
      double bandCenter = band0Start + bandwidthBand / 2.0;
      
      NS_LOG_LOGIC ("OVERLAPPING, FDD: "
                    << bandwidthBand << ":" << bandwidthCc << ":"
                    << bandwidthBwp << ", "
                    << (int)numCcPerBand << ", " << (int)numBwp);

      NS_LOG_LOGIC ("bandConf0: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf0 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf0.m_numBwp = numBwp;
      bandCenter += bandwidthBand;
      
      // Create, then configure
      CcBwpCreator ccBwpCreator;
      band0 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf0); 
      band0.m_bandId = 0;
          
      bandCenter = band0Start + bandwidthBwp / 2.0;
      
      NS_LOG_LOGIC ("band0[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);
      bandCenter += bandwidthBwp;
      
      NS_LOG_LOGIC ("band0[0][1]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[1], bandCenter, bandwidthBwp);

      std::cout << "BWP Configuration for OVERLAPPING case, mode "
                << operationMode << "\n" << band0;
    }

  else if ( (freqScenario == 1) // OVERLAPPING
            && (operationMode == "TDD"))
    {
      // Use double with BWP, to match total bandwidth for FDD in UL and DL
      bandwidthBwp *= 2;
      
      uint8_t numBwp = 1;
      double bandwidthCc = numBwp * bandwidthBwp;
      uint8_t numCcPerBand = 1;
      double bandwidthBand = numCcPerBand * bandwidthCc;
      double bandCenter = band0Start + bandwidthBand / 2.0;
      
      NS_LOG_LOGIC ("NON_OVERLAPPING, TDD: "
                    << bandwidthBand << ":" << bandwidthCc << ":"
                    << bandwidthBwp << ", "
                    << (int)numCcPerBand << ", " << (int)numBwp);
      
      NS_LOG_LOGIC ("bandConf0: " << bandCenter << " " << bandwidthBand);
      CcBwpCreator::SimpleOperationBandConf
        bandConf0 (bandCenter, bandwidthBand, numCcPerBand, scene);
      bandConf0.m_numBwp = numBwp;
      bandCenter += bandwidthBand;

      // Create, then configure
      CcBwpCreator ccBwpCreator;
      band0 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf0);
      band0.m_bandId = 0;
          
      bandCenter = band0Start + bandwidthBwp / 2.0;
      
      NS_LOG_LOGIC ("band0[0][0]: " << bandCenter << " " << bandwidthBwp);
      ConfigureBwpTo (band0.m_cc[0]->m_bwp[0], bandCenter, bandwidthBwp);

      std::cout << "BWP Configuration for OVERLAPPING case, mode "
                << operationMode << "\n" << band0;
    }

  else
    {
      std::cerr << "unknown combination of freqScenario = " << freqScenario
                << " and operationMode = " << operationMode
                << std::endl;
      exit (1);
    }
      

  auto bandMask = NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL;
  // Omit fading from calibration mode
  if (! calibration)
    {
      bandMask |= NrHelper::INIT_FADING;
    }
  nrHelper->InitializeOperationBand (&band0, bandMask);
  nrHelper->InitializeOperationBand (&band1, bandMask);
  nrHelper->InitializeOperationBand (&band2, bandMask);

  BandwidthPartInfoPtrVector sector1Bwps, sector2Bwps, sector3Bwps;
  if (freqScenario == 0) // NON_OVERLAPPING
    {
      sector1Bwps = CcBwpCreator::GetAllBwps ({band0});
      sector2Bwps = CcBwpCreator::GetAllBwps ({band1});
      sector3Bwps = CcBwpCreator::GetAllBwps ({band2});
    }
  else // OVERLAPPING
    {
      sector1Bwps = CcBwpCreator::GetAllBwps ({band0});
      sector2Bwps = CcBwpCreator::GetAllBwps ({band0});
      sector3Bwps = CcBwpCreator::GetAllBwps ({band0});
    }

  /*
   * Start to account for the bandwidth used by the example, as well as
   * the total power that has to be divided among the BWPs. Since we are TDD
   * or FDD with 2 BWP only, there is no need to divide anything.
   */
  double totalTxPower = txPowerBs; //Convert to mW
  double x = pow (10, totalTxPower / 10);

  /*
   * Now, we can setup the attributes. We can have three kind of attributes:
   * (i) parameters that are valid for all the bandwidth parts and applies to
   * all nodes, (ii) parameters that are valid for all the bandwidth parts
   * and applies to some node only, and (iii) parameters that are different for
   * every bandwidth parts. The approach is:
   *
   * - for (i): Configure the attribute through the helper, and then install;
   * - for (ii): Configure the attribute through the helper, and then install
   * for the first set of nodes. Then, change the attribute through the helper,
   * and install again;
   * - for (iii): Install, and then configure the attributes by retrieving
   * the pointer needed, and calling "SetAttribute" on top of such pointer.
   *
   */

  /*
   *  Case (i): Attributes valid for all the nodes
   */
  // Beamforming method

  if (radioNetwork == "LTE" && calibration == true)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
    }
  else if (radioNetwork == "NR")
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    }

  // Scheduler type

  if (scheduler == "PF")
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));
    }
  else if (scheduler == "RR")
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaRR"));
    }

  nrHelper->SetSchedulerAttribute ("DlCtrlSymbols", UintegerValue (1));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));
  nrHelper->SetUeAntennaAttribute ("ElementGain", DoubleValue (0));

  // Antennas for all the gNbs
  if (calibration)
    {
      nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
      nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (5));
      nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (2));
    }

  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (false));
  nrHelper->SetGnbAntennaAttribute ("ElementGain", DoubleValue (0));
  nrHelper->SetGnbAntennaAttribute ("DowntiltAngle",
                                    DoubleValue (downtiltAngle * M_PI / 180.0));

  // UE transmit power
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (23.0));

  // Set LTE RBG size
  // TODO: What these values would be in TDD? bandwidthMhz refers to FDD.
  // for example, for TDD, if we have bandwidthMhz to 20, we will have a 40 MHz
  // BWP.
  if (radioNetwork == "LTE")
    {
      if (bandwidthMHz == 20)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (bandwidthMHz == 15)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (bandwidthMHz == 10)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (3));
        }
      else if (bandwidthMHz == 5)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (2));
        }
      else
        {
          NS_ABORT_MSG ("Currently, only supported bandwidths are 5, 10, 15, and 20MHz, you chose " << bandwidthMHz);
        }
    }

  // We assume a common traffic pattern for all UEs
  uint32_t bwpIdForLowLat = 0;
  if (operationMode == "FDD" && direction == "UL")
    {
      bwpIdForLowLat = 1;
    }

  // gNb routing between Bearer and bandwidth part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForLowLat));

  // Ue routing between Bearer and bandwidth part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForLowLat));

  /*
   * We miss many other parameters. By default, not configuring them is equivalent
   * to use the default values. Please, have a look at the documentation to see
   * what are the default values for all the attributes you are not seeing here.
   */

  /*
   * Case (ii): Attributes valid for a subset of the nodes
   */

  // NOT PRESENT IN THIS SIMPLE EXAMPLE

  /*
   * We have configured the attributes we needed. Now, install and get the pointers
   * to the NetDevices, which contains all the NR stack:
   */

  //  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gridScenario.GetBaseStations (), allBwps);
  gnbSector1NetDev = nrHelper->InstallGnbDevice (gnbSector1Container, sector1Bwps);
  gnbSector2NetDev = nrHelper->InstallGnbDevice (gnbSector2Container, sector2Bwps);
  gnbSector3NetDev = nrHelper->InstallGnbDevice (gnbSector3Container, sector3Bwps);
  ueSector1NetDev = nrHelper->InstallUeDevice (ueSector1Container, sector1Bwps);
  ueSector2NetDev = nrHelper->InstallUeDevice (ueSector2Container, sector2Bwps);
  ueSector3NetDev = nrHelper->InstallUeDevice (ueSector3Container, sector3Bwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gnbSector1NetDev, randomStream);
  randomStream += nrHelper->AssignStreams (gnbSector2NetDev, randomStream);
  randomStream += nrHelper->AssignStreams (gnbSector3NetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueSector1NetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueSector2NetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueSector3NetDev, randomStream);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Sectors (cells) of a site are pointing at different directions
  double orientationRads = gridScenario.GetAntennaOrientationRadians (0, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector1NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector1NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (3, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (3, 30);

          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (3, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (1, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector2NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector2NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (2, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal.
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (2, 30);

          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (2, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (2, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector3NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector3NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (0, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (0, 30);

          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (0, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }


  // Set the UE routing:

  if (operationMode == "FDD")
    {
      for (uint32_t i = 0; i < ueSector1NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector1NetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueSector2NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector2NetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueSector3NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector3NetDev.Get (i))->SetOutputLink (0, 1);
        }
    }

  for (uint32_t i = 0; i < ueSector1NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector1NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector1NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
    }

  for (uint32_t i = 0; i < ueSector2NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector2NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector2NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
    }

  for (uint32_t i = 0; i < ueSector3NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector3NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector3NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
    }

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = gnbSector1NetDev.Begin (); it != gnbSector1NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbSector2NetDev.Begin (); it != gnbSector2NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbSector3NetDev.Begin (); it != gnbSector3NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector1NetDev.Begin (); it != ueSector1NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector2NetDev.Begin (); it != ueSector2NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector3NetDev.Begin (); it != ueSector3NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }
}

} // namespace ns3
