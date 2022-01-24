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
 * \file nr-prose-relay-mcptt.cc
 * \ingroup examples
 *
 * \brief Basic scenario with some UEs doing in-network communication and some
 *  out-of-network UEs (remote UEs) doing in-network communication through
 *  ProSe L3 UE-to-Network (U2N) relay UEs.
 *
 * TODO: Double check we are covering all input parameters and outputs before
 *       releasing.
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
 * The scenario is composed of one gNB and configurable number of in-network
 * only UEs, in-network relay UEs and out-of-network remote UEs.
 * The total number of in-network only UEs can be configured using the
 * 'nInNetUes' parameter (default value = 1). The total number of in-network
 * relay UEs can be configured using the 'nRelayUes' parameter (default
 * value = 1). The number of remote UEs connected to each relay UE can be
 * configured using the parameter 'nRemoteUesPerRelay' (default value = 1),
 * for a total of nRelayUes * nRemoteUesPerRelay remote UEs in the scenario.
 * The in-network only UEs and the in-network relay UEs are located over the
 * circumference of circles centered in the gNB. The parameter 'radiusInNetUes'
 * controls the radio of the circumference for the in-network only UEs (default
 * value = 10 m), and the parameter 'radiusRelayUes' does the same for the
 * in-network relay UEs (default value = 15 m). The remote UEs are located over
 * the circumference of a circle centered in their corresponding relay UE,
 * which radius is given by the parameter 'radiusRemoteUes' (default
 * value = 15 m).
 *
 * ProSe L3 UE-to-Network relay
 * The remote UEs will start the establishment of the L3 U2N relay connection
 * before the start of the in-network traffic. This will internally start the
 * establishment of the corresponding ProSe unicast direct links with their
 * corresponding relay UEs.
 *
 *
 * Traffic:
 * The traffic generation is controlled by a in-network MCPTT system that
 * comprises several McpttPttApps and a McpttServerApp:
 * - McpttPttApps are installed in each UE in the system
 * - The McpttServerApp is installed in a IMS node in the network.
 * There are two groups of UEs in the scenario:
 * - Group 1 composed of the In-network UEs (in-network only UEs and relay UEs)
 * - Group 2 composed of the remote UEs.
 * The UEs of a group participate in the same MCPTT call, hence there are two
 * calls ongoing in the scenario.
 * Each call has a McpttPusherOrchestrator that controls when each of the
 * McpttPttApp on the call "pushes the button" according to a contention
 * probability to be set by the 'cp' parameter (default cp = 0) and a session
 * activity factor to be set by the 'saf' parameter (default  saf = 1.0).
 * The McpttPttApp generates media traffic for a duration that follows a given
 * distribution (if granted the floor after pushing the button). This can be
 * controlled with the parameter 'vaf' (voice activity factor, default vaf = 1.0)
 *
 *
 * Output:
 * The example will print on-screen the traffic flows configuration and the
 * end-to-end statistics of each of them after the simulation finishes.
 * Additionally, several output files are generated:
 * 1. nr-prose-relay-mcptt-flowMonitorOutput.txt: contain the statistics
 * printed on the standard output.
 * 2. nr-prose-relay-mcptt-netsimulyzer.json: json file to be used to
 * visualize the simulation un the NetSimulyzer. This file is generated only if
 * the netsimulyzer module is present in the ns3 tree.
 * 3. nr-prose-relay-mcptt-topology.plt: gnuplot script to plot the topology
 * of the scenario. Users can generate the topology plot by running:
 *   \code{.unparsed}
$ gnuplot nr-prose-relay-mcptt-topology.plt
    \endcode
 * 4. nr-prose-relay-mcptt-traces.db: sqlite3 database containing
 * Sidelink MAC and PHY layer traces of the UEs doing SL
 * 5. NrSlPc5SignallingPacketTrace.txt: log of the transmitted and received PC5
 * signaling messages for the UEs using the SL.
 * 6. NrSlRelayNasRxPacketTrace.txt: Log of the data packets relayed by the
 * relay UEs.
 * 7. mcptt-m2e-latency.txt: MCPTT mouth-to-ear latency trace
 * 8. mcptt-access-time.txt: MCPTT access time trace
 * 9. mcptt-msg-stats.txt: MCPTT messages trace
 * 10. mcptt-state-machine-stats.txt: MCPTT state machine trace
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
#include "ns3/psc-module.h"
#include "ns3/sip-module.h"


#ifdef HAS_NETSIMULYZER
#include <ns3/netsimulyzer-module.h>
#endif

#include <sqlite3.h>
//Dependency of these nr-v2x-examples classes for SL statistics
#include "../nr-v2x-examples/ue-mac-pscch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-mac-pssch-tx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pscch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-phy-pssch-rx-output-stats.h"
#include "../nr-v2x-examples/ue-to-ue-pkt-txrx-output-stats.h"
#include "../nr-v2x-examples/ue-rlc-rx-output-stats.h"

using namespace ns3;
using namespace psc;

NS_LOG_COMPONENT_DEFINE ("NrProseRelayOnOff");

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

/************** END Methods for tracing SL using databases *******************/


/*
 * Trace sink function for logging transmission and reception of PC5 signaling (PC5-S) messages
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
 * Trace sink function for logging reception of data packets in the NAS layer by UE(s) acting as relay UE
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

#ifdef HAS_NETSIMULYZER
/*
* Structure to store the NetSimulizer traces of a MCPTT flow
* A MCPTT flow: One transmitter and all receivers in the group
*/
struct NetSimulyzerMcpttFlow
{

  void TxTrace (Ptr<const Application> app, uint16_t callId, Ptr<const Packet> p, const TypeId& headerType);
  void RxTrace (Ptr<const Application> app, uint16_t callId, Ptr<const Packet> p, const TypeId& headerType);

  uint32_t m_txNodeId; ///< Node ID of the transmitter
  Ptr<netsimulyzer::ThroughputSink> m_txTputSeries; ///< Curve for the transmitted throughput
  std::map <uint32_t, Ptr<netsimulyzer::ThroughputSink> > m_rxTputSeriesPerRx; ///< Set of curves for the received throughput from the transmitter, one per receiver
  std::map <uint32_t, Ptr<netsimulyzer::XYSeries> > m_rxDelaySeriesPerRx; ///< Set of curves for the media packet delay from the transmitter, one per receiver
  std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> > m_rxDelayEcdfPerRx; ///< Set of eCDFs for the media packet delay from the transmitter, one per receiver

};

/*
* Trace sink function to trace media packets transmissions and add them to the
* appropriate netsimulizer series
*/
void
NetSimulyzerMcpttFlow::TxTrace (Ptr<const Application> app, uint16_t callId, Ptr<const Packet> p, const TypeId& headerType)
{
  if (headerType == McpttMediaMsg::GetTypeId ())
    {
      m_txTputSeries->AddPacket (p);
    }
}

/*
* Trace sink function to trace media packet receptions and add them to the
* appropriate netsimulizer series
*/
void
NetSimulyzerMcpttFlow::RxTrace (Ptr<const Application> app, uint16_t callId, Ptr<const Packet> p, const TypeId& headerType)
{
  if (headerType == McpttMediaMsg::GetTypeId ())
    {
      McpttMediaMsg msg;
      p->PeekHeader (msg);

      uint32_t txingNodeId = msg.GetSsrc ();
      uint32_t thisNodeId = app->GetNode ()->GetId ();

      if (m_txNodeId == txingNodeId)
        {
          //Troughput
          //Find ThroughputSink
          std::map <uint32_t, Ptr<netsimulyzer::ThroughputSink> >::iterator it = m_rxTputSeriesPerRx.find (thisNodeId);
          if (it != m_rxTputSeriesPerRx.end ())
            {
              it->second->AddPacket (p);
            }
          else
            {
              NS_FATAL_ERROR ("Rx not found?!");
            }

          //Delay
          McpttRtpHeader rtpHeader = msg.GetHeader ();
          Time timeStamp = MicroSeconds (rtpHeader.GetTimestamp () * 10); //RTP timestamp impl is in 10ths of microseconds
          double delay = Simulator::Now ().GetSeconds () * 1000.0  - timeStamp.GetSeconds () * 1000.0; //ms

          //Find delay XYSeries
          std::map <uint32_t, Ptr<netsimulyzer::XYSeries> >::iterator it2 = m_rxDelaySeriesPerRx.find (thisNodeId);
          if (it2 != m_rxDelaySeriesPerRx.end ())
            {
              it2->second->Append (Simulator::Now ().GetSeconds (), delay);
            }
          else
            {
              NS_FATAL_ERROR ("Rx not found?!");
            }

          //Find delay eCDF
          std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> >::iterator it3 = m_rxDelayEcdfPerRx.find (thisNodeId);
          if (it3 != m_rxDelayEcdfPerRx.end ())
            {
              it3->second->Append (delay);
            }
          else
            {
              NS_FATAL_ERROR ("Rx not found?!");
            }
        }
    }
}


struct NetSimulyzerMcpttMetricsTracer
{

  void AccessTimeTrace (Time t, uint32_t userId, uint16_t callId, std::string result, Time latency);
  void M2eLatencyTrace (Time t, uint32_t ssrc, uint64_t nodeId, uint16_t callId, Time latency);

  std::map <uint32_t, Ptr<netsimulyzer::XYSeries> > m_m2eLatencyTimelineSeriesPerCall; ///< Set of M2E latency timeline curves, one per call
  std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> > m_m2eLatencyEcdfPerCall; ///< Set of M2E eCDFs, one per call
  std::map <uint32_t, Ptr<netsimulyzer::XYSeries> > m_accessTimeTimelineSeriesPerCall; ///< Set of M2E timeline curves, one per call
  std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> > m_accessTimeEcdfPerCall; ///< Set of M2E eCDFs, one per call

};
/*
 * Function to trace the MCPTT access time
 */
void
NetSimulyzerMcpttMetricsTracer::AccessTimeTrace (Time t, uint32_t userId, uint16_t callId, std::string result, Time latency)
{
  std::map <uint32_t, Ptr<netsimulyzer::XYSeries> >::iterator it = m_accessTimeTimelineSeriesPerCall.find (callId);
  if (it != m_accessTimeTimelineSeriesPerCall.end ())
    {
      it->second->Append (Simulator::Now ().GetSeconds (), latency.GetMilliSeconds ());
    }
  else
    {
      NS_FATAL_ERROR ("Call not found in tracer?!");
    }
  std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> >::iterator it2 = m_accessTimeEcdfPerCall.find (callId);
  if (it2 != m_accessTimeEcdfPerCall.end ())
    {
      it2->second->Append (latency.GetMilliSeconds ());
    }
  else
    {
      NS_FATAL_ERROR ("Call not found in tracer?!");
    }
}

/*
 * Function to trace the MCPTT mouth to ear latency
 */
void
NetSimulyzerMcpttMetricsTracer::M2eLatencyTrace (Time t, uint32_t ssrc, uint64_t nodeId, uint16_t callId, Time latency)
{
  std::map <uint32_t, Ptr<netsimulyzer::XYSeries> >::iterator it = m_m2eLatencyTimelineSeriesPerCall.find (callId);
  if (it != m_m2eLatencyTimelineSeriesPerCall.end ())
    {
      it->second->Append (Simulator::Now ().GetSeconds (), latency.GetMilliSeconds ());
    }
  else
    {
      NS_FATAL_ERROR ("Call not found in tracer?!");
    }
  std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> >::iterator it2 = m_m2eLatencyEcdfPerCall.find (callId);
  if (it2 != m_m2eLatencyEcdfPerCall.end ())
    {
      it2->second->Append (latency.GetMilliSeconds ());
    }
  else
    {
      NS_FATAL_ERROR ("Call not found in tracer?!");
    }
}



/*
 * Structure containing the colors used for the netSimulyzer curves
 */
static const std::vector<netsimulyzer::Color3Value> g_colors
{
  netsimulyzer::DARK_RED_VALUE,
  netsimulyzer::DARK_GREEN_VALUE,
  netsimulyzer::DARK_BLUE_VALUE,
  netsimulyzer::DARK_ORANGE_VALUE,
  netsimulyzer::DARK_PURPLE_VALUE,
  netsimulyzer::DARK_YELLOW_VALUE,
  netsimulyzer::DARK_PINK_VALUE,
  netsimulyzer::BLACK,
  netsimulyzer::RED_VALUE,
  netsimulyzer::GREEN_VALUE,
  netsimulyzer::BLUE_VALUE,
  netsimulyzer::ORANGE_VALUE,
  netsimulyzer::PURPLE_VALUE,
  netsimulyzer::YELLOW_VALUE,
  netsimulyzer::PINK_VALUE
};

uint32_t g_idxColor = 0;  ///< Index used for choosing the next netSimulyzer color

/*
 * Function to obtain the next color used for the netSimulyzer curves
 */
netsimulyzer::Color3Value GetNextColor ()
{
  netsimulyzer::Color3Value returnValue = g_colors[g_idxColor];
  g_idxColor++;
  if (g_idxColor > g_colors.size () - 1)
    {
      g_idxColor = 0;
    }
  return returnValue;
}

#endif


/**
 * Function that generates a gnuplot script file that can be used to plot the
 * topology of the scenario access network (eNBs, Relay UEs and Remote UEs)
 */
void
GenerateTopologyPlotFile (std::string fileNameWithNoExtension, NodeContainer gNbNode, NodeContainer inNetUeNodes, NodeContainer relayUeNodes, NodeContainer remoteUeNodes,
                          double relayRadius, double remoteRadius )
{
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string gnuplotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "Topology (Labels = Node IDs)";

  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);
  plot.SetTerminal ("png size 1024,1024");
  plot.SetLegend ("X", "Y"); //These are the axis, not the legend
  std::ostringstream plotExtras;
  plotExtras << "set xrange [-" << 1.1 * (relayRadius + remoteRadius) << ":+" << 1.1 * (relayRadius + remoteRadius) << "]" << std::endl;
  plotExtras << "set yrange [-" << 1.1 * (relayRadius + remoteRadius) << ":+" << 1.1 * (relayRadius + remoteRadius) << "]" << std::endl;
  plotExtras << "set linetype 1 pt 3 ps 2 " << std::endl;
  plotExtras << "set linetype 2 lc rgb \"green\" pt 2 ps 2" << std::endl;
  plotExtras << "set linetype 3 pt 1 ps 2" << std::endl;
  plot.AppendExtra (plotExtras.str ());

  //eNB
  Gnuplot2dDataset datasetEnodeB;
  datasetEnodeB.SetTitle ("eNodeB");
  datasetEnodeB.SetStyle (Gnuplot2dDataset::POINTS);

  double x = gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().x;
  double y = gNbNode.Get (0)->GetObject<MobilityModel> ()->GetPosition ().y;
  std::ostringstream strForLabel;
  strForLabel << "set label \"" << gNbNode.Get (0)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
  plot.AppendExtra (strForLabel.str ());
  datasetEnodeB.Add (x, y);
  plot.AddDataset (datasetEnodeB);

  //InNet UEs
  Gnuplot2dDataset datasetInNets;
  datasetInNets.SetTitle ("InNetwork UEs");
  datasetInNets.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t inNetIdx = 0; inNetIdx < inNetUeNodes.GetN (); inNetIdx++)
    {
      double x = inNetUeNodes.Get (inNetIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = inNetUeNodes.Get (inNetIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << inNetUeNodes.Get (inNetIdx)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetInNets.Add (x, y);
    }
  plot.AddDataset (datasetInNets);

  //Relay UEs
  Gnuplot2dDataset datasetRelays;
  datasetRelays.SetTitle ("Relay UEs");
  datasetRelays.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ryIdx++)
    {
      double x = relayUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = relayUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << relayUeNodes.Get (ryIdx)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
      plot.AppendExtra (strForLabel.str ());
      datasetRelays.Add (x, y);
    }
  plot.AddDataset (datasetRelays);

  //Remote UEs
  Gnuplot2dDataset datasetRemotes;
  datasetRemotes.SetTitle ("Remote UEs");
  datasetRemotes.SetStyle (Gnuplot2dDataset::POINTS);
  for (uint32_t rmIdx = 0; rmIdx < remoteUeNodes.GetN (); rmIdx++)
    {
      double x = remoteUeNodes.Get (rmIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = remoteUeNodes.Get (rmIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      std::ostringstream strForLabel;
      strForLabel << "set label \"" << remoteUeNodes.Get (rmIdx)->GetId () << "\" at " << x << "," << y << " textcolor rgb \"grey\" center front offset 0,1";
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
  double centralFrequencyBand = 5.89e9; // band n47 (From SL examples)
  double bandwidthBand = 40e6; //40 MHz
  double centralFrequencyCc0 = 5.89e9;
  double bandwidthCc0 = bandwidthBand;
  std::string pattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|"; // From SL examples
  double bandwidthCc0Bpw0 = bandwidthCc0 / 2;
  double bandwidthCc0Bpw1 = bandwidthCc0 / 2;
  double ueTxPower = 23; //dBm

  //In-network devices configuration
  uint16_t numerologyCc0Bwp0 = 2; // BWP0 will be used for the in-network
  double gNBtotalTxPower = 32; // dBm
  bool cellScan = false;  // Beamforming method.
  double beamSearchAngleStep = 10.0; // Beamforming parameter.

  //Sidelink configuration
  uint16_t numerologyCc0Bwp1 = 2; //(From SL examples)  BWP1 will be used for SL
  Time startRelayConnTime = Seconds (2.0); //Time to start the U2N relay connection establishment
  bool enableSensing = false;

  //Topology
  uint16_t nInNetUes = 1;
  uint16_t nRelayUes = 1;
  uint16_t nRemoteUesPerRelay = 2;
  uint16_t nRemoteUes = nRelayUes * nRemoteUesPerRelay;
  uint16_t radiusInNetUes = 10; //meters
  uint16_t radiusRelayUes = 15; //meters
  uint16_t radiusRemoteUes = 4; //meters
  double gNbHeight = 10; //meters
  double ueHeight = 1.5; //meters

  //Traffic
  Time timeStartTraffic = Seconds (5.0);
  bool relayUesTraffic = true;

  //MCPTT configuration
  double cp = 0.0; // Pusher orchestrator Contention Probability
  double vaf = 1.0; // Pusher orchestrator Voice Activity Factor
  double saf = 1.0; // Pusher orchestrator Session Activity Factor

  //Simulation configuration
  std::string outputDir = "./";
  std::string exampleName = "nr-prose-relay-mcptt";

  Time simTime = Seconds (40.0); // seconds

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation (s)", simTime);
  cmd.AddValue ("nInNetUes", "Number of in-network only UEs", nInNetUes);
  cmd.AddValue ("nRelayUes", "Number of relay UEs", nRelayUes);
  cmd.AddValue ("nRemoteUesPerRelay", "Number of remote UEs per relay UE", nRemoteUesPerRelay);
  cmd.AddValue ("radiusInNetUes", "Radius of the circle (centered in the gNB) where in-network only UEs are positioned", radiusInNetUes);
  cmd.AddValue ("radiusRelayUes", "Radius of the circle (centered in the gNB) where relay UEs are positioned", radiusRelayUes);
  cmd.AddValue ("radiusRemoteUes", "Radius of the circle (centered in the relay UE) where remote UEs are positioned", radiusRemoteUes);
  cmd.AddValue ("relayUesTraffic", "True if relay UEs have their own data traffic", relayUesTraffic);
  cmd.AddValue ("cp", "The pusher orchestrator contention probability.", cp);
  cmd.AddValue ("vaf", "The pusher orchestrator voice activity factor.", vaf);
  cmd.AddValue ("saf", "The pusher orchestrator session activity factor.", saf);
  cmd.AddValue ("enableSensing", "True if sensing is activated", enableSensing);

  cmd.Parse (argc, argv);


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
  inNetUeNodes.Create (nInNetUes);
  relayUeNodes.Create (nRelayUes);
  remoteUeNodes.Create (nRelayUes * nRemoteUesPerRelay);

  Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();
  gNbPositionAlloc->Add (Vector (0.0, 0.0, gNbHeight));

  //UEs
  Ptr<ListPositionAllocator> posAllocInNet = CreateObject<ListPositionAllocator> ();
  for (uint32_t inNetIdx = 0; inNetIdx < inNetUeNodes.GetN (); ++inNetIdx)
    {
      //Relay UE
      double in_angle = 45.0 + inNetIdx * (360.0 / inNetUeNodes.GetN ()); //degrees
      double in_pos_x = radiusInNetUes * std::cos (in_angle * M_PI / 180.0);
      double in_pos_y = radiusInNetUes * std::sin (in_angle * M_PI / 180.0);

      posAllocInNet->Add (Vector (in_pos_x, in_pos_y, ueHeight));
    }

  Ptr<ListPositionAllocator> posAllocRelays = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> posAllocRemotes = CreateObject<ListPositionAllocator> ();
  for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
    {
      //Relay UE
      double ry_angle = ryIdx * (360.0 / relayUeNodes.GetN ()); //degrees
      double ry_pos_x = radiusRelayUes * std::cos (ry_angle * M_PI / 180.0);
      double ry_pos_y = radiusRelayUes * std::sin (ry_angle * M_PI / 180.0);

      posAllocRelays->Add (Vector (ry_pos_x, ry_pos_y, ueHeight));

      NS_LOG_INFO ("Relay UE " << ryIdx + 1 << " node id = [" << relayUeNodes.Get (ryIdx)->GetId () << "]"
                   " x " << ry_pos_x << " y " << ry_pos_y);
      //Remote UEs
      for (uint32_t rmIdx = 0; rmIdx < nRemoteUesPerRelay; ++rmIdx)
        {
          double rm_angle = 90.0 + rmIdx * (360.0 / nRemoteUesPerRelay); //degrees
          double rm_pos_x = ry_pos_x + radiusRemoteUes * std::cos (rm_angle * M_PI / 180.0);
          double rm_pos_y = ry_pos_y + radiusRemoteUes * std::sin (rm_angle * M_PI / 180.0);

          posAllocRemotes->Add (Vector (rm_pos_x, rm_pos_y, ueHeight));

          uint32_t remoteIdx = ryIdx * nRemoteUesPerRelay + rmIdx;
          NS_LOG_INFO ("Remote UE " << remoteIdx << " node id = [" << remoteUeNodes.Get (remoteIdx)->GetId () << "]"
                       " x " << rm_pos_x << " y " << rm_pos_y);
        }
    }

  //Install mobility
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (posAllocInNet);
  mobility.Install (inNetUeNodes);

  mobility.SetPositionAllocator (posAllocRelays);
  mobility.Install (relayUeNodes);

  mobility.SetPositionAllocator (posAllocRemotes);
  mobility.Install (remoteUeNodes);

  //Generate gnuplot file with the script to generate the topology plot
  std::string topoFilenameWithNoExtension = exampleName + "-topology";
  GenerateTopologyPlotFile (topoFilenameWithNoExtension, gNbNodes, inNetUeNodes, relayUeNodes, remoteUeNodes, radiusRelayUes, radiusRemoteUes);


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
  nrHelper->SetUeMacAttribute ("ReservationPeriod", TimeValue (MilliSeconds (10)));
  nrHelper->SetUeMacAttribute ("NumSidelinkProcess", UintegerValue (255)); //TODO: I was 4, I increased it because we hit an error where no HARQ processes were available
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
  nrSlHelper->SetNrSlSchedulerTypeId (NrSlUeMacSchedulerNist::GetTypeId ());
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
  // get SGW/PGW
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  InternetStackHelper internet;

  //Create the IMS server to be used as the MCPTT server
  Ptr<psc::ImsHelper> imsHelper = CreateObject<psc::ImsHelper> ();
  imsHelper->ConnectPgw (pgw);


  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");

  std::cout << "IP configuration: " << std::endl;

  // Configure in-network only UEs
  internet.Install (inNetUeNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (inNetUeNetDev));
  std::vector<Ipv4Address> inNetIpv4AddressVector (nInNetUes);

  // Set the default gateway for the in-network UEs
  for (uint32_t j = 0; j < inNetUeNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (inNetUeNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      inNetIpv4AddressVector [j] = inNetUeNodes.Get (j)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      std::cout << " In-network UE: " << inNetIpv4AddressVector [j] << std::endl;

    }

  //Attach in-network UEs to the closest gNB
  nrHelper->AttachToClosestEnb (inNetUeNetDev, enbNetDev);

  // Configure U2N relay UEs
  internet.Install (relayUeNodes);
  Ipv4InterfaceContainer ueIpIfaceRelays;
  ueIpIfaceRelays = epcHelper->AssignUeIpv4Address (NetDeviceContainer (relayUeNetDev));
  std::vector<Ipv4Address> relaysIpv4AddressVector (nRelayUes);

  for (uint32_t u = 0; u < relayUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (relayUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      relaysIpv4AddressVector [u] = relayUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      std::cout << " Relay UE: " << relaysIpv4AddressVector [u] << std::endl;
    }

  //Attach U2N relay UEs to the closest gNB
  nrHelper->AttachToClosestEnb (relayUeNetDev, enbNetDev);

  // Configure out-of-network UEs
  internet.Install (remoteUeNodes);
  Ipv4InterfaceContainer ueIpIfaceSl;
  ueIpIfaceSl = epcHelper->AssignUeIpv4Address (NetDeviceContainer (remoteUeNetDev));
  std::vector<Ipv4Address> remotesIpv4AddressVector (nRemoteUes);

  for (uint32_t u = 0; u < remoteUeNodes.GetN (); ++u)
    {
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (remoteUeNodes.Get (u)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      //Obtain local IPv4 addresses that will be used to route the unicast traffic upon setup of the direct link
      remotesIpv4AddressVector [u] = remoteUeNodes.Get (u)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      std::cout << " Remote UE: " << remotesIpv4AddressVector [u] << std::endl;
    }


  /******** Configure ProSe layer in the UEs that will do SL  **********/
  //Create ProSe helper
  Ptr<NrSlProseHelper> nrSlProseHelper = CreateObject <NrSlProseHelper> ();
  nrSlProseHelper->SetEpcHelper (epcHelper);

  // Install ProSe layer and corresponding SAPs in the UES
  nrSlProseHelper->PrepareUesForProse (relayUeNetDev);
  nrSlProseHelper->PrepareUesForProse (remoteUeNetDev);

  //Configure ProSe Unicast parameters. At the moment it only instruct the MAC
  //layer (and PHY therefore) to monitor packets directed the UE's own Layer 2 ID
  nrSlProseHelper->PrepareUesForUnicast (relayUeNetDev);
  nrSlProseHelper->PrepareUesForUnicast (remoteUeNetDev);

  //Configure the value of timer Timer T5080 (Prose Direct Link Establishment Request Retransmission)
  //to a lower value than the standard (8.0 s) to speed connection in shorter simulation time
  Config::SetDefault ("ns3::NrSlUeProseDirectLink::T5080", TimeValue (Seconds (2.0)));

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
  NS_LOG_INFO ("Configuring remote UE - relay UE connection..." );
  for (uint32_t i = 0; i < remoteUeNodes.GetN (); ++i)
    {
      for (uint32_t j = 0; j < relayUeNetDev.GetN (); ++j)
        {
          nrSlProseHelper->EstablishL3UeToNetworkRelayConnection (startRelayConnTime,
                                                                  remoteUeNetDev.Get (i), remotesIpv4AddressVector [i], // Remote UE
                                                                  relayUeNetDev.Get (j), relaysIpv4AddressVector [j], // Relay UE
                                                                  relayServiceCode);

          NS_LOG_INFO ("Remote UE nodeId " << i << " Relay UE nodeId " << j);
        }
    }

  /******************** END L3 U2N Relay configuration ***********************/


#ifdef HAS_NETSIMULYZER

  std::string netSimOutputFilename = exampleName + "-netsimulyzer.json";
  auto orchestrator = CreateObject<netsimulyzer::Orchestrator> (netSimOutputFilename);
  orchestrator->SetAttribute ("PollMobility", BooleanValue (false)); // The Nodes don't move during the simulation, so disable mobility polling
  orchestrator->SetAttribute ("TimeStep", netsimulyzer::OptionalValue<int>{100});

  netsimulyzer::NodeConfigurationHelper nodeHelper {
    orchestrator
  };

  //Nodes for 3D scene
  nodeHelper.Set ("Model", netsimulyzer::models::SMARTPHONE_VALUE);
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::GREEN});
  for (uint32_t i = 0; i < inNetUeNodes.GetN (); ++i)
    {
      nodeHelper.Set ("Name", StringValue ("InNet UE - Node " + std::to_string (inNetUeNodes.Get (i)->GetId () )));
      nodeHelper.Install (inNetUeNodes.Get (i));
    }
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::BLUE});
  for (uint32_t i = 0; i < relayUeNodes.GetN (); ++i)
    {
      nodeHelper.Set ("Name", StringValue ("Relay UE - Node " + std::to_string (relayUeNodes.Get (i)->GetId () )));
      nodeHelper.Install (relayUeNodes.Get (i));
    }
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{netsimulyzer::PURPLE});
  for (uint32_t i = 0; i < remoteUeNodes.GetN (); ++i)
    {
      nodeHelper.Set ("Name", StringValue ("Remote UE - Node " + std::to_string (remoteUeNodes.Get (i)->GetId () )));
      nodeHelper.Install (remoteUeNodes.Get (i));
    }

  nodeHelper.Set ("Model", netsimulyzer::models::CELL_TOWER_POLE_VALUE);
  nodeHelper.Set ("Height", netsimulyzer::OptionalValue<double> (10));
  nodeHelper.Set ("Name", StringValue ("gNB"));
  nodeHelper.Install (gNbNodes);

  PointerValue xAxis;
  PointerValue yAxis;
  uint16_t groupVectorsSize = 3; //size 3 because we ignore group 0


  std::vector <std::map <uint32_t, NetSimulyzerMcpttFlow> > netSimulyzerFlowsPerGroup (groupVectorsSize);
  std::vector <std::vector <uint32_t> > nodeIdsPerGroup (groupVectorsSize);
  for (uint32_t inIdx = 0; inIdx < inNetUeNodes.GetN (); ++inIdx)
    {
      nodeIdsPerGroup[1].emplace_back (inNetUeNodes.Get (inIdx)->GetId ());

    }
  for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
    {
      nodeIdsPerGroup[1].emplace_back (relayUeNodes.Get (ryIdx)->GetId ());

    }
  for (uint32_t rmIdx = 0; rmIdx < remoteUeNodes.GetN (); ++rmIdx)
    {
      nodeIdsPerGroup[2].emplace_back (remoteUeNodes.Get (rmIdx)->GetId ());

    }

  //Create netsimulizer collections to be maintained per group
  std::vector <Ptr<netsimulyzer::SeriesCollection> > rxDelayEcdfCollectionsPerGroup (groupVectorsSize);
  std::vector <Ptr<netsimulyzer::SeriesCollection> > rxDelayTimelineCollectionsPerGroup (groupVectorsSize);
  std::vector <Ptr<netsimulyzer::SeriesCollection> > txTputTimelineCollectionsPerGroup (groupVectorsSize);
  std::vector <Ptr<netsimulyzer::SeriesCollection> > rxTputTimelineCollectionsPerGroup (groupVectorsSize);

  for (uint32_t groupId = 1; groupId < groupVectorsSize; ++groupId)
    {

      txTputTimelineCollectionsPerGroup[groupId] = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      txTputTimelineCollectionsPerGroup[groupId]->SetAttribute ("Name", StringValue ("Throughput - Timeline - Tx - CallId "
                                                                                     + std::to_string (groupId)));
      txTputTimelineCollectionsPerGroup[groupId]->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      txTputTimelineCollectionsPerGroup[groupId]->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
      txTputTimelineCollectionsPerGroup[groupId]->SetAttribute ("HideAddedSeries", BooleanValue (true));

      rxTputTimelineCollectionsPerGroup[groupId] = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      rxTputTimelineCollectionsPerGroup[groupId]->SetAttribute ("Name", StringValue ("Throughput - Timeline - Rx - CallId "
                                                                                     + std::to_string (groupId)));
      rxTputTimelineCollectionsPerGroup[groupId]->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      rxTputTimelineCollectionsPerGroup[groupId]->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
      rxTputTimelineCollectionsPerGroup[groupId]->SetAttribute ("HideAddedSeries", BooleanValue (true));

      rxDelayTimelineCollectionsPerGroup[groupId] = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      rxDelayTimelineCollectionsPerGroup[groupId]->SetAttribute ("Name", StringValue ("Media packet delay - Timeline - CallId "
                                                                                      + std::to_string (groupId)));
      rxDelayTimelineCollectionsPerGroup[groupId]->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      rxDelayTimelineCollectionsPerGroup[groupId]->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Media packet delay (ms)"));
      rxDelayTimelineCollectionsPerGroup[groupId]->SetAttribute ("HideAddedSeries", BooleanValue (true));

      rxDelayEcdfCollectionsPerGroup[groupId] = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      rxDelayEcdfCollectionsPerGroup[groupId]->SetAttribute ("Name", StringValue ("Media packet delay - eCDF - CallId "
                                                                                  + std::to_string (groupId)));
      rxDelayEcdfCollectionsPerGroup[groupId]->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Media packet delay (ms)"));
      rxDelayEcdfCollectionsPerGroup[groupId]->SetAttribute ("HideAddedSeries", BooleanValue (true));
    }

  //Create netsimulizer series and collections per flow
  for (uint32_t groupId = 1; groupId < groupVectorsSize; ++groupId)
    {
      //First add all transmitters -> create flows and setup transmitter trace for all UEs in group
      for (auto& txNodeId: nodeIdsPerGroup[groupId])
        {
          NetSimulyzerMcpttFlow flow;
          flow.m_txNodeId = txNodeId;
          flow.m_txTputSeries = CreateObject<netsimulyzer::ThroughputSink> (orchestrator,
                                                                            "Call ID " + std::to_string (groupId) +
                                                                            " - Flow Tx - Tx Id " + std::to_string (txNodeId));
          flow.m_txTputSeries->GetSeries ()->SetAttribute ("Color", GetNextColor ());
          netSimulyzerFlowsPerGroup[groupId].insert (std::pair<uint32_t, NetSimulyzerMcpttFlow> (txNodeId, flow));
        }
      //Then each UE adds itself as a receiver to all transmitter in the group  -> setup receiver traces
      for (auto& rxNodeId: nodeIdsPerGroup[groupId])
        {
          for (std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it = netSimulyzerFlowsPerGroup[groupId].begin ();
               it != netSimulyzerFlowsPerGroup[groupId].end (); ++it)
            {
              if (it->first == rxNodeId)
                {
                  continue;
                }
              //Rx throughput
              auto rxTputSeries = CreateObject<netsimulyzer::ThroughputSink> (orchestrator,
                                                                              "Call ID " + std::to_string (groupId) +
                                                                              " - Flow Rx - Tx Id "
                                                                              + std::to_string (it->first)
                                                                              + " - Rx Id " + std::to_string (rxNodeId));
              rxTputSeries->GetSeries ()->SetAttribute ("Color", GetNextColor ());
              it->second.m_rxTputSeriesPerRx.insert (std::pair<uint32_t, Ptr<netsimulyzer::ThroughputSink> > (rxNodeId,rxTputSeries ));

              //Media packet delay Timeline
              auto rxDelaySeries = CreateObject <netsimulyzer::XYSeries> (orchestrator);
              rxDelaySeries->SetAttribute ("Name", StringValue ("Call ID " + std::to_string (groupId) +
                                                                " - Flow Rx - Tx Id "
                                                                + std::to_string (it->first)
                                                                + " - Rx Id " + std::to_string (rxNodeId)));
              rxDelaySeries->SetAttribute ("Color", GetNextColor ());
              rxDelaySeries->SetAttribute ("Connection", StringValue ("None"));
              rxDelaySeries->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
              rxDelaySeries->GetYAxis ()->SetAttribute ("Name", StringValue ("Media packet delay (ms)"));
              it->second.m_rxDelaySeriesPerRx.insert (std::pair<uint32_t, Ptr<netsimulyzer::XYSeries> > (rxNodeId,rxDelaySeries));

              //Media packet delay eCDF
              auto rxDelayEcdf = CreateObject<netsimulyzer::EcdfSink> (orchestrator,"Call ID " + std::to_string (groupId) +
                                                                       " - Flow Rx - Tx Id "
                                                                       + std::to_string (it->first)
                                                                       + " - Rx Id " + std::to_string (rxNodeId));
              rxDelayEcdf->GetXAxis ()->SetAttribute ("Name", StringValue ("Media packet delay"));
              rxDelayEcdf->GetSeries ()->SetAttribute ("Color", GetNextColor ());
              it->second.m_rxDelayEcdfPerRx.insert (std::pair<uint32_t, Ptr<netsimulyzer::EcdfSink> > (rxNodeId,rxDelayEcdf));
            }
        }

      // netsimulyzer collections for metrics per flow
      for (std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it = netSimulyzerFlowsPerGroup[groupId].begin ();
           it != netSimulyzerFlowsPerGroup[groupId].end (); ++it)
        {

          //Collection for Tx throughput and corresponding Rx throughput series of this flow
          auto tputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
          tputCollection->SetAttribute ("Name", StringValue ("Throughput - Timeline - CallId " + std::to_string (groupId) +
                                                             " - Tx Id " + std::to_string (it->first)));
          tputCollection->GetAttribute ("XAxis", xAxis);
          xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
          tputCollection->GetAttribute ("YAxis", yAxis);
          yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));


          tputCollection->Add (it->second.m_txTputSeries->GetSeries ());
          txTputTimelineCollectionsPerGroup[groupId]->Add (it->second.m_txTputSeries->GetSeries ());
          for (std::map <uint32_t, Ptr<netsimulyzer::ThroughputSink> >::iterator it2 = it->second.m_rxTputSeriesPerRx.begin ();
               it2 != it->second.m_rxTputSeriesPerRx.end (); ++it2)
            {
              tputCollection->Add (it2->second->GetSeries ());
              rxTputTimelineCollectionsPerGroup[groupId]->Add (it2->second->GetSeries ());
            }

          //Collection for Rx packet delay of this flow - timeline
          auto delaySeriesCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
          delaySeriesCollection->SetAttribute ("Name", StringValue ("Media packet delay - Timeline - CallId " + std::to_string (groupId) +
                                                                    " - Tx Id " + std::to_string (it->first)));
          delaySeriesCollection->GetAttribute ("XAxis", xAxis);
          xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
          delaySeriesCollection->GetAttribute ("YAxis", yAxis);
          yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Media packet delay (ms)"));

          for (std::map <uint32_t, Ptr<netsimulyzer::XYSeries> >::iterator it3 = it->second.m_rxDelaySeriesPerRx.begin ();
               it3 != it->second.m_rxDelaySeriesPerRx.end (); ++it3)
            {
              delaySeriesCollection->Add (it3->second);
              rxDelayTimelineCollectionsPerGroup[groupId]->Add (it3->second);
            }

          //Collection for Rx packet delay of this flow - eCDF
          auto delayEcdfCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
          delayEcdfCollection->SetAttribute ("Name", StringValue ("Media packet delay - eCDF - CallId " + std::to_string (groupId) +
                                                                  " - Tx Id " + std::to_string (it->first)));
          delayEcdfCollection->GetAttribute ("XAxis", xAxis);
          xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Media packet delay (ms)"));

          for (std::map <uint32_t, Ptr<netsimulyzer::EcdfSink> >::iterator it4 = it->second.m_rxDelayEcdfPerRx.begin ();
               it4 != it->second.m_rxDelayEcdfPerRx.end (); ++it4)
            {
              delayEcdfCollection->Add (it4->second->GetSeries ());
              rxDelayEcdfCollectionsPerGroup[groupId]->Add (it4->second->GetSeries ());
            }
        }
    }
#endif

  /************************ Application configuration ************************/
  /* In-network MCPPT application comprises McpttPttApp and McpttServerApp
   * - McpttPttApps are installed in each UE
   * - The McpttServerApp is installed in the IMS node.
   * There are two groups of UEs in the scenario:
   * - Group 1 composed of the In-network UEs (In-network UEs and Relay UEs) and,
   * - Group 2 composed of the Remote UEs.
   * The UEs of a group participate in the same MCPTT call, hence there are two
   * calls ongoing in the scenario.
   * Each call has a McpttPusherOrchestrator that controls when each of the
   * McpttPttApp on the call "pushes the button". The McpttPttApp generates
   * media traffic if granted the floor for a duration controlled by the
   * orchestrator.
  */


  //MCPTT server configuration
  ApplicationContainer serverApps;
  psc::McpttServerHelper mcpttServerHelper;

  //Install MCPTT sever in the IMS node
  serverApps.Add (mcpttServerHelper.Install (imsHelper->GetImsNode ()));

  //Configure IP address of the MCPTT server
  Ptr<psc::McpttServerApp> serverApp = DynamicCast<psc::McpttServerApp> (serverApps.Get (0));
  Ipv4Address serverAddress = Ipv4Address::ConvertFrom (imsHelper->GetImsGmAddress ());
  serverApp->SetLocalAddress (serverAddress);
  std::cout << " IMS node (MCPTT Server): " << serverAddress << std::endl;


  //MCPTT clients configuration
  psc::McpttHelper mcpttClientHelper;
  mcpttClientHelper.SetPttApp ("ns3::psc::McpttPttApp");
  mcpttClientHelper.SetMediaSrc ("ns3::psc::McpttMediaSrc",
                                 "Bytes", UintegerValue (60),
                                 "DataRate", DataRateValue (DataRate ("24kb/s")));


  std::cout << "InNetwork UEs traffic flows: " << std::endl;
  uint32_t groupId = 1;
  ApplicationContainer inNetClientApps;
  for (uint32_t inIdx = 0; inIdx < inNetUeNodes.GetN (); ++inIdx)
    {
      //Install MCPTT client the UE
      ApplicationContainer pttAppContainer = mcpttClientHelper.Install (inNetUeNodes.Get (inIdx));
      Ptr<psc::McpttPttApp> pttApp = pttAppContainer.Get (0)->GetObject<psc::McpttPttApp> ();
      pttApp->SetLocalAddress (inNetIpv4AddressVector[inIdx]);

#ifdef HAS_NETSIMULYZER
      //Tracing with netSimulyzer
      uint32_t nodeId = inNetUeNodes.Get (inIdx)->GetId ();
      //Connect transmission trace source to this UE
      std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it = netSimulyzerFlowsPerGroup[groupId].find (nodeId);
      pttApp->TraceConnectWithoutContext ("TxTrace", MakeCallback (&NetSimulyzerMcpttFlow::TxTrace, &it->second));

      //Connect reception trace source to the other transmitters in the group
      for (std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it2 = netSimulyzerFlowsPerGroup[groupId].begin ();
           it2 != netSimulyzerFlowsPerGroup[groupId].end (); ++it2)
        {
          if (it2->first == nodeId)
            {
              continue;
            }
          else
            {
              pttApp->TraceConnectWithoutContext ("RxTrace", MakeCallback (&NetSimulyzerMcpttFlow::RxTrace, &it2->second));
            }
        }
#endif

      inNetClientApps.Add (pttApp);

      std::cout << " UL: " << inNetIpv4AddressVector [inIdx] << " -> " << serverAddress  << std::endl;
      std::cout << " DL: " << serverAddress << " -> " << inNetIpv4AddressVector [inIdx] << std::endl;

      //DL bearer configuration
      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      tftDl->Add (pfDl);
      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerDl, tftDl);

      //UL bearer configuration
      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remoteAddress = serverAddress; //IMPORTANT!!!
      tftUl->Add (pfUl);
      enum EpsBearer::Qci qUl;
      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerUl, tftUl);
    }

  //Relay UEs
  ApplicationContainer relayClientApps;
  if (relayUesTraffic)
    {
      std::cout << "Relay UEs traffic flows: " << std::endl;
      groupId = 1;
      for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
        {
          ApplicationContainer pttAppContainer = mcpttClientHelper.Install (relayUeNodes.Get (ryIdx));
          Ptr<psc::McpttPttApp> pttApp = pttAppContainer.Get (0)->GetObject<psc::McpttPttApp> ();
          pttApp->SetLocalAddress (relaysIpv4AddressVector[ryIdx]);
#ifdef HAS_NETSIMULYZER
          //Tracing with netSimulyzer
          uint32_t nodeId = relayUeNodes.Get (ryIdx)->GetId ();
          //Connect transmission trace source to this UE
          std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it = netSimulyzerFlowsPerGroup[groupId].find (nodeId);
          pttApp->TraceConnectWithoutContext ("TxTrace", MakeCallback (&NetSimulyzerMcpttFlow::TxTrace, &it->second));

          //Connect reception trace source to the other transmitters in the group
          for (std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it2 = netSimulyzerFlowsPerGroup[groupId].begin ();
               it2 != netSimulyzerFlowsPerGroup[groupId].end (); ++it2)
            {
              if (it2->first == nodeId)
                {
                  continue;
                }
              else
                {
                  pttApp->TraceConnectWithoutContext ("RxTrace", MakeCallback (&NetSimulyzerMcpttFlow::RxTrace, &it2->second));
                }
            }
#endif

          relayClientApps.Add (pttApp);

          std::cout << " UL: " << relaysIpv4AddressVector [ryIdx] << " -> " << serverAddress  << std::endl;
          std::cout << " DL: " << serverAddress << " -> " << relaysIpv4AddressVector [ryIdx] << std::endl;

          //DL bearer configuration
          Ptr<EpcTft> tftDl = Create<EpcTft> ();
          EpcTft::PacketFilter pfDl;
          tftDl->Add (pfDl);
          enum EpsBearer::Qci qDl;
          qDl = EpsBearer::GBR_CONV_VOICE;
          EpsBearer bearerDl (qDl);
          nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerDl, tftDl);

          //UL bearer configuration
          Ptr<EpcTft> tftUl = Create<EpcTft> ();
          EpcTft::PacketFilter pfUl;
          pfUl.remoteAddress = serverAddress; //IMPORTANT!!!
          tftUl->Add (pfUl);
          enum EpsBearer::Qci qUl;
          qUl = EpsBearer::GBR_CONV_VOICE;
          EpsBearer bearerUl (qUl);
          nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerUl, tftUl);
        }

    }

  //Remote UEs
  std::cout << "Remote UEs traffic flows: " << std::endl;
  groupId = 2;
  ApplicationContainer remoteClientApps;
  for (uint32_t rmIdx = 0; rmIdx < remoteUeNodes.GetN (); ++rmIdx)
    {
      //Install MCPTT client the UE
      ApplicationContainer pttAppContainer = mcpttClientHelper.Install (remoteUeNodes.Get (rmIdx));
      Ptr<psc::McpttPttApp> pttApp = pttAppContainer.Get (0)->GetObject<psc::McpttPttApp> ();
      pttApp->SetLocalAddress (remotesIpv4AddressVector[rmIdx]);

#ifdef HAS_NETSIMULYZER
      //Tracing with netSimulyzer
      uint32_t nodeId = remoteUeNodes.Get (rmIdx)->GetId ();
      //Connect transmission trace source to this UE
      std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it = netSimulyzerFlowsPerGroup[groupId].find (nodeId);
      pttApp->TraceConnectWithoutContext ("TxTrace", MakeCallback (&NetSimulyzerMcpttFlow::TxTrace, &it->second));

      //Connect reception trace source to the other transmitters in the group
      for (std::map <uint32_t, NetSimulyzerMcpttFlow>::iterator it2 = netSimulyzerFlowsPerGroup[groupId].begin ();
           it2 != netSimulyzerFlowsPerGroup[groupId].end (); ++it2)
        {
          if (it2->first == nodeId)
            {
              continue;
            }
          else
            {
              pttApp->TraceConnectWithoutContext ("RxTrace", MakeCallback (&NetSimulyzerMcpttFlow::RxTrace, &it2->second));
            }
        }
#endif

      remoteClientApps.Add (pttApp);

      std::cout << " UL: " << remotesIpv4AddressVector [rmIdx] << " -> " << serverAddress  << std::endl;
      std::cout << " DL: " << serverAddress << " -> " << remotesIpv4AddressVector [rmIdx] << std::endl;

      //DL bearer configuration
      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      tftDl->Add (pfDl);
      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (remoteUeNetDev.Get (rmIdx), bearerDl, tftDl);

      //UL bearer configuration
      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remoteAddress = serverAddress; //IMPORTANT!!!
      tftUl->Add (pfUl);
      enum EpsBearer::Qci qUl;
      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (remoteUeNetDev.Get (rmIdx), bearerUl, tftUl);
    }

  //MCPTT client orchestrator configuration
  //Group 1
  Ptr<psc::McpttPusherOrchestratorInterface> groupOneMcpttOrchestrator = 0;
  Ptr<psc::McpttPusherOrchestratorSpurtCdf> groupOneSpurtOrchestrator = CreateObject<psc::McpttPusherOrchestratorSpurtCdf> ();
  groupOneSpurtOrchestrator->SetAttribute ("ActivityFactor", DoubleValue (vaf));
  groupOneMcpttOrchestrator = groupOneSpurtOrchestrator;

  Ptr<psc::McpttPusherOrchestratorContention> groupOneContentionOrchestrator = CreateObject<psc::McpttPusherOrchestratorContention> ();
  groupOneContentionOrchestrator->SetAttribute ("ContentionProbability", DoubleValue (cp));
  groupOneContentionOrchestrator->SetAttribute ("Orchestrator", PointerValue (groupOneMcpttOrchestrator));
  groupOneMcpttOrchestrator = groupOneContentionOrchestrator;

  Ptr<psc::McpttPusherOrchestratorSessionCdf> groupOneSessionOrchestrator = CreateObject<psc::McpttPusherOrchestratorSessionCdf> ();
  groupOneSessionOrchestrator->SetAttribute ("ActivityFactor", DoubleValue (saf));
  groupOneSessionOrchestrator->SetAttribute ("Orchestrator", PointerValue (groupOneMcpttOrchestrator));
  groupOneMcpttOrchestrator = groupOneSessionOrchestrator;

  groupOneMcpttOrchestrator->StartAt (timeStartTraffic);
  groupOneMcpttOrchestrator->StopAt (simTime - Seconds (1.0));

  //Group 2
  Ptr<psc::McpttPusherOrchestratorInterface> groupTwoMcpttOrchestrator = 0;
  Ptr<psc::McpttPusherOrchestratorSpurtCdf> groupTwoSpurtOrchestrator = CreateObject<psc::McpttPusherOrchestratorSpurtCdf> ();
  groupTwoSpurtOrchestrator->SetAttribute ("ActivityFactor", DoubleValue (vaf));
  groupTwoMcpttOrchestrator = groupTwoSpurtOrchestrator;

  Ptr<psc::McpttPusherOrchestratorContention> groupTwoContentionOrchestrator = CreateObject<psc::McpttPusherOrchestratorContention> ();
  groupTwoContentionOrchestrator->SetAttribute ("ContentionProbability", DoubleValue (cp));
  groupTwoContentionOrchestrator->SetAttribute ("Orchestrator", PointerValue (groupTwoMcpttOrchestrator));
  groupTwoMcpttOrchestrator = groupTwoContentionOrchestrator;

  Ptr<psc::McpttPusherOrchestratorSessionCdf> groupTwoSessionOrchestrator = CreateObject<psc::McpttPusherOrchestratorSessionCdf> ();
  groupTwoSessionOrchestrator->SetAttribute ("ActivityFactor", DoubleValue (saf));
  groupTwoSessionOrchestrator->SetAttribute ("Orchestrator", PointerValue (groupTwoMcpttOrchestrator));
  groupTwoMcpttOrchestrator = groupTwoSessionOrchestrator;

  groupTwoMcpttOrchestrator->StartAt (timeStartTraffic);
  groupTwoMcpttOrchestrator->StopAt (simTime - Seconds (1.0));

  std::map<uint32_t, std::vector<Ipv4Address> > ipv4addressPerGroup;
  std::vector<Ipv4Address> groupOneIpv4Addresses;
  std::vector<Ipv4Address> groupTwoIpv4Addresses;

  ApplicationContainer groupOneClientApps;
  groupOneClientApps.Add (inNetClientApps);
  groupOneClientApps.Add (relayClientApps);
  groupOneIpv4Addresses.insert (groupOneIpv4Addresses.begin (), inNetIpv4AddressVector.begin (),inNetIpv4AddressVector.end ());
  groupOneIpv4Addresses.insert (groupOneIpv4Addresses.begin (), relaysIpv4AddressVector.begin (),relaysIpv4AddressVector.end ());

  ApplicationContainer groupTwoClientApps;
  groupTwoClientApps.Add (remoteClientApps);
  groupTwoIpv4Addresses.insert (groupTwoIpv4Addresses.begin (), remotesIpv4AddressVector.begin (),remotesIpv4AddressVector.end ());

  ApplicationContainer allClientApps;
  allClientApps.Add (groupOneClientApps);
  allClientApps.Add (groupTwoClientApps);

  //Connect MCPTT client apps with the orchestrator
  mcpttClientHelper.AddPushersToOrchestrator (groupOneMcpttOrchestrator, groupOneClientApps);
  mcpttClientHelper.AddPushersToOrchestrator (groupTwoMcpttOrchestrator, groupTwoClientApps);

  //Configure MCPTT call
  psc::McpttCallHelper callHelper;
  // Optional statements to tailor the configurable attributes
  callHelper.SetArbitrator ("ns3::psc::McpttOnNetworkFloorArbitrator",
                            "AckRequired", BooleanValue (false),
                            "AudioCutIn", BooleanValue (false),
                            "DualFloorSupported", BooleanValue (false),
                            "QueueingSupported", BooleanValue (true));
  callHelper.SetTowardsParticipant ("ns3::psc::McpttOnNetworkFloorTowardsParticipant",
                                    "ReceiveOnly", BooleanValue (false));
  callHelper.SetParticipant ("ns3::psc::McpttOnNetworkFloorParticipant",
                             "AckRequired", BooleanValue (false),
                             "GenMedia", BooleanValue (true));
  callHelper.SetServerCall ("ns3::psc::McpttServerCall",
                            "AmbientListening", BooleanValue (false),
                            "TemporaryGroup", BooleanValue (false));

  groupId = 1;
  callHelper.AddCall (groupOneClientApps, serverApp, groupId,
                      psc::McpttCallMsgFieldCallType::BASIC_GROUP,
                      timeStartTraffic, simTime - Seconds (1.0));
  ipv4addressPerGroup.insert (std::pair < uint32_t, std::vector<Ipv4Address> > (groupId, groupOneIpv4Addresses));

  groupId = 2;
  callHelper.AddCall (groupTwoClientApps, serverApp, groupId,
                      psc::McpttCallMsgFieldCallType::BASIC_GROUP,
                      timeStartTraffic, simTime - Seconds (1.0));

  ipv4addressPerGroup.insert (std::pair < uint32_t, std::vector<Ipv4Address> > (groupId, groupTwoIpv4Addresses));


  allClientApps.Start (timeStartTraffic - Seconds (0.4));
  allClientApps.Stop (simTime - Seconds (1.0));
  serverApps.Start (timeStartTraffic - Seconds (0.6));
  serverApps.Stop (simTime - Seconds (1.0));
  /******************** End Application configuration ************************/

  /**************** MCPTT metrics tracing ************************************/

  Ptr<McpttTraceHelper> mcpttTraceHelper = CreateObject<McpttTraceHelper> ();
  mcpttTraceHelper->EnableMsgTraces ();
  mcpttTraceHelper->EnableStateMachineTraces ();
  mcpttTraceHelper->EnableMouthToEarLatencyTrace ("mcptt-m2e-latency.txt");
  mcpttTraceHelper->EnableAccessTimeTrace ("mcptt-access-time.txt");

#ifdef HAS_NETSIMULYZER

  //Collection to show eCDFs of MCPTT metrics per group together
  auto accessTimeEcdfCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  accessTimeEcdfCollection->SetAttribute ("Name", StringValue ("Access time - eCDF - All calls"));
  accessTimeEcdfCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Access time (ms)"));
  accessTimeEcdfCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Empirical CDF"));
  accessTimeEcdfCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));

  auto m2eLatencyEcdfCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  m2eLatencyEcdfCollection->SetAttribute ("Name", StringValue ("M2E latency - eCDF - All calls"));
  m2eLatencyEcdfCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("M2E latency (ms)"));
  m2eLatencyEcdfCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Empirical CDF"));
  m2eLatencyEcdfCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));

  //Collection to show Timelines of MCPTT metrics per group together
  auto accessTimeTimelineCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  accessTimeTimelineCollection->SetAttribute ("Name", StringValue ("Access time - Timeline - All calls"));
  accessTimeTimelineCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
  accessTimeTimelineCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Access time (ms)"));
  accessTimeTimelineCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));

  auto m2eLatencyTimelineCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  m2eLatencyTimelineCollection->SetAttribute ("Name", StringValue ("M2E latency - Timeline - All calls"));
  m2eLatencyTimelineCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
  m2eLatencyTimelineCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("M2E latency (ms)"));
  m2eLatencyTimelineCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));


  //Connect traces to the MCPTT trace helper and add them to the respective collections
  NetSimulyzerMcpttMetricsTracer mcpttMetricsTracer;

  for (uint16_t callId = 1; callId < groupVectorsSize; ++callId) //callId == groupId
    {
      //ECDFs for Access time
      auto groupAccessTimeEcdf = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Access time - eCDF - CallId " + std::to_string (callId));
      groupAccessTimeEcdf->GetXAxis ()->SetAttribute ("Name", StringValue ("Access time (ms)"));
      groupAccessTimeEcdf->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      mcpttMetricsTracer.m_accessTimeEcdfPerCall.insert (std::pair<uint32_t, Ptr<netsimulyzer::EcdfSink> > (callId, groupAccessTimeEcdf) );
      accessTimeEcdfCollection->Add (groupAccessTimeEcdf->GetSeries ());

      //ECDFs for M2E latency
      auto groupM2eLatencyEcdfs = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "M2E latency - eCDF - CallId " + std::to_string (callId));
      groupM2eLatencyEcdfs->GetXAxis ()->SetAttribute ("Name", StringValue ("M2E latency (ms)"));
      groupM2eLatencyEcdfs->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      mcpttMetricsTracer.m_m2eLatencyEcdfPerCall.insert (std::pair<uint32_t, Ptr<netsimulyzer::EcdfSink> > (callId, groupM2eLatencyEcdfs) );
      m2eLatencyEcdfCollection->Add (groupM2eLatencyEcdfs->GetSeries ());

      //Timeline for access time
      auto  groupAccessTimeTimeline = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      groupAccessTimeTimeline->SetAttribute ("Name", StringValue ("Access time - Timeline - CallId " + std::to_string (callId)));
      groupAccessTimeTimeline->SetAttribute ("Color", GetNextColor ());
      groupAccessTimeTimeline->SetAttribute ("Connection", StringValue ("None"));
      groupAccessTimeTimeline->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      groupAccessTimeTimeline->GetYAxis ()->SetAttribute ("Name", StringValue ("Access time (ms)"));
      mcpttMetricsTracer.m_accessTimeTimelineSeriesPerCall.insert (std::pair<uint32_t, Ptr<netsimulyzer::XYSeries> > (callId, groupAccessTimeTimeline) );
      accessTimeTimelineCollection->Add (groupAccessTimeTimeline);

      //Timeline for M2E latency
      auto groupM2eLatencyTimeline = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      groupM2eLatencyTimeline->SetAttribute ("Name", StringValue ("M2E latency - Timeline - CallId " + std::to_string (callId)));
      groupM2eLatencyTimeline->SetAttribute ("Color", GetNextColor ());
      groupM2eLatencyTimeline->SetAttribute ("Connection", StringValue ("None"));
      groupM2eLatencyTimeline->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      groupM2eLatencyTimeline->GetYAxis ()->SetAttribute ("Name", StringValue ("M2E latency (ms)"));
      mcpttMetricsTracer.m_m2eLatencyTimelineSeriesPerCall.insert (std::pair<uint32_t, Ptr<netsimulyzer::XYSeries> > (callId, groupM2eLatencyTimeline) );
      m2eLatencyTimelineCollection->Add (groupM2eLatencyTimeline);
    }

  mcpttTraceHelper->TraceConnectWithoutContext ("AccessTimeTrace",
                                                MakeCallback (&NetSimulyzerMcpttMetricsTracer::AccessTimeTrace, &mcpttMetricsTracer));
  mcpttTraceHelper->TraceConnectWithoutContext ("MouthToEarLatencyTrace",
                                                MakeCallback (&NetSimulyzerMcpttMetricsTracer::M2eLatencyTrace, &mcpttMetricsTracer));

#endif


  /**************** END MCPTT metrics tracing ************************************/


  /************ SL traces database setup *************************************/
  SQLiteOutput db (outputDir + exampleName + "-traces.db", exampleName);

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
  UeRlcRxOutputStats ueRlcRxStats;
  ueRlcRxStats.SetDb (&db, "rlcRx");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/RxRlcPduWithTxRnti",
                                 MakeBoundCallback (&NotifySlRlcPduRx, &ueRlcRxStats));

  /************ END SL traces database setup *************************************/


  AsciiTraceHelper ascii;

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
  /*************** END Received messages by the relay tracing **************/


  // nrHelper->EnableTraces ();

  //Configure FlowMonitor to get traffic flow statistics
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (inNetUeNodes);
  endpointNodes.Add (remoteUeNodes);
  endpointNodes.Add (relayUeNodes);
  endpointNodes.Add (imsHelper->GetImsNode ());

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  //Run simulation
  Simulator::Stop (simTime);

  /*
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();
  outputConfig.ConfigureDefaults ();
   */
  Simulator::Run ();

  //SL database dump
  pscchStats.EmptyCache ();
  psschStats.EmptyCache ();
  pscchPhyStats.EmptyCache ();
  psschPhyStats.EmptyCache ();
  ueRlcRxStats.EmptyCache ();

  //Print per-network-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  std::ofstream outFile;
  std::string filename = outputDir + "/" + exampleName  + "-flowMonitorOutput.txt";
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);
  for (std::map<uint32_t, std::vector<Ipv4Address> >::const_iterator itGroupAddr = ipv4addressPerGroup.begin (); itGroupAddr != ipv4addressPerGroup.end (); ++itGroupAddr)
    {
      outFile << "#Type\tcallId\tsrcIpd\tdstIp\tnTxPackets\tnRxPackets\tLossRatio\tMeanDelay(ms)\tMeanJitter(ms)\n";

      for (std::vector<Ipv4Address>::const_iterator itAddr = itGroupAddr->second.begin (); itAddr != itGroupAddr->second.end (); itAddr++)
        {
          //Media packets first
          for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
            {
              Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

              if (t.sourceAddress == *itAddr || t.destinationAddress == *itAddr)
                {
                  if (t.sourcePort == 5060 ||  t.sourcePort % 2 == 0 )
                    {
                      continue; //We are looking for Media packets, continue to next flow.
                      outFile << "Sign." << "\t";
                    }
                  else
                    {
                      outFile << "Media" << "\t";
                    }

                  outFile << itGroupAddr->first << "\t"; //groupId
                  outFile << t.sourceAddress << ":" << t.sourcePort << "\t"
                          << t.destinationAddress << ":" << t.destinationPort << "\t";

                  outFile << i->second.txPackets << "\t";
                  outFile << i->second.rxPackets << "\t";
                  outFile << (double) (i->second.txPackets - i->second.rxPackets) / i->second.txPackets << "\t";
                  //outFile << i->second.txBytes << "\t";
                  //outFile << i->second.rxBytes << "\t";
                  if (i->second.rxPackets > 0)
                    {
                      outFile << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << "\t";
                      outFile << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << "\n";
                    }
                  else
                    {
                      outFile << "0" << "\t";
                      outFile << "0" << "\n";
                    }
                }
            }
        }
      for (std::vector<Ipv4Address>::const_iterator itAddr = itGroupAddr->second.begin (); itAddr != itGroupAddr->second.end (); itAddr++)
        {
          //Signalling packets after
          for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
            {
              Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

              if (t.sourceAddress == *itAddr || t.destinationAddress == *itAddr)
                {
                  if (t.sourcePort == 5060 ||  t.sourcePort % 2 == 0 )
                    {
                      outFile << "Sign." << "\t";
                    }
                  else
                    {
                      continue; //We are looking for Signalling packets, continue to next flow.
                      outFile << "Media" << "\t";
                    }

                  outFile << itGroupAddr->first << "\t"; //groupId
                  outFile << t.sourceAddress << ":" << t.sourcePort << "\t"
                          << t.destinationAddress << ":" << t.destinationPort << "\t";

                  outFile << i->second.txPackets << "\t";
                  outFile << i->second.rxPackets << "\t";
                  outFile << (double) (i->second.txPackets - i->second.rxPackets) / i->second.txPackets << "\t";
                  //outFile << i->second.txBytes << "\t";
                  //outFile << i->second.rxBytes << "\t";
                  if (i->second.rxPackets > 0)
                    {
                      outFile << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << "\t";
                      outFile << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << "\n";
                    }
                  else
                    {
                      outFile << "0" << "\t";
                      outFile << "0" << "\n";
                    }
                }
            }
        }
    }
  outFile.close ();

  std::cout << "Simulation done!"  << std::endl << "Traffic flows statistics: " << std::endl;
  std::ifstream f (filename.c_str ());
  if (f.is_open ())
    {
      std::cout << f.rdbuf ();
    }
  std::cout << "Number of packets relayed by the L3 UE-to-Network relays:"  << std::endl;
  std::cout << "relayIp      srcIp->dstIp      srcLink->dstLink\t\tnPackets"  << std::endl;
  for (auto it = g_relayNasPacketCounter.begin (); it != g_relayNasPacketCounter.end (); ++it)
    {
      std::cout << it->first << "\t\t" << it->second << std::endl;
    }



  Simulator::Destroy ();
  return 0;
}


