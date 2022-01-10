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
 * \file nr-prose-relay-on-off.cc
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
 * There is traffic flowing in both directions (UL and DL) for the in-network
 * only UEs and the remote UEs. The parameter 'relayUesTraffic' controls
 * whether the relay UE has its own traffic as well (default value = true).
 * Each UE has two traffic flows: one from a Remote Host in the internet
 * towards the UE (DL), and one from the UEs towards the Remote Host (UL).
 * Each traffic flow is controlled by an OnOffApplication that generate
 * packets of a fixed size at a constant bit rate during the 'On' periods,
 * and no packets during the 'Off' periods. The duration of those periods are
 * controlled by random variables with hardcoded parameters.
 * TODO: give the user the power. fixed duration, or random with given parameters.
 *
 * Output:
 * The example will print on-screen the traffic flows configuration and the
 * end-to-end statistics of each of them after the simulation finishes.
 * Additionally, several output files are generated:
 * 1. nr-prose-relay-on-off-flowMonitorOutput.txt: contain the statistics
 * printed on the standard output.
 * 2. nr-prose-relay-on-off-netsimulyzer.json: json file to be used to
 * visualize the simulation un the NetSimulyzer. This file is generated only if
 * the netsimulyzer module is present in the ns3 tree.
 * 3. nr-prose-relay-on-off-topology.plt: gnuplot script to plot the topology
 * of the scenario. Users can generate the topology plot by running:
 *   \code{.unparsed}
$ gnuplot nr-prose-relay-on-off-topology.plt
    \endcode
 * 4. nr-prose-relay-on-off-traces.db: sqlite3 database containing
 * Sidelink MAC and PHY layer traces as well as application layer traces of the
 * UEs doing SL
 * 5. NrSlPc5SignallingPacketTrace.txt: log of the transmitted and received PC5
 * signaling messages for the UEs using the SL.
 * 6. NrSlAppRxPacketDelayTrace.txt: Log of the application layer packet delay.
 * 7. NrSlRelayNasRxPacketTrace.txt: Log of the data packets relayed by the
 * relay UEs.
 *
 *
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
std::map<std::string, PacketWithRxTimestamp> g_rxPacketsForDelayCalc;

/*
 * Trace sink function for logging the transmitted data packets and their
 * corresponding transmission timestamp at the application layer
 */
void
TxPacketTraceForDelay (const Address &localAddrs, Ptr<const Packet> p, const Address &srcAddrs,
                       const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
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
  g_rxPacketsForDelayCalc.insert (std::pair < std::string, PacketWithRxTimestamp> (mapKey, mapValue));

}

/*
 * Trace sink function for calculating the packet delay upon reception at the
 * application layer
 */
void
RxPacketTraceForDelay (Ptr<OutputStreamWrapper> stream, Ptr<Node> node, const Address &localAddrs, Ptr<const Packet> p, const Address &srcAddrs,
                       const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  double delay = 0.0;
  std::ostringstream  oss;
  oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
      << "->"
      << Ipv4Address::ConvertFrom (localAddrs)
      << "("
      << seqTsSizeHeader.GetSeq ()
      << ")";
  std::string mapKey = oss.str ();
  auto it = g_rxPacketsForDelayCalc.find (mapKey);
  if (it == g_rxPacketsForDelayCalc.end ())
    {
      NS_FATAL_ERROR ("Rx packet not found?!");
    }
  else
    {
      delay = Simulator::Now ().GetSeconds () * 1000.0 - it->second.txTimestamp.GetSeconds () * 1000.0;
    }
  *stream->GetStream () << Simulator::Now ().GetSeconds ()
                        << "\t" << node->GetId ()
                        << "\t" << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
                        << "\t" << Ipv4Address::ConvertFrom (localAddrs)
                        << "\t" << seqTsSizeHeader.GetSeq ()
                        << "\t" << delay
                        << std::endl;
}

#ifdef HAS_NETSIMULYZER
/*
 * Trace sink to add packets to a netSimulyzer ThroughputSink upon transmission
 * and reception of packets at the application layer
 */
void
PacketTraceNetSimulyzer (Ptr<netsimulyzer::ThroughputSink> sink, std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
                         const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  sink->AddPacket (p);
}
/*
 * Trace sink to calculate the packet delay upon reception at the
 * application layer and add it to a netSimulyzer XYSeries.
 */
void
RxPacketTraceForDelayNetSimulyzer (Ptr<netsimulyzer::XYSeries> series,
                                   Ptr<Node> node,
                                   const Address &localAddrs,
                                   Ptr<const Packet> p,
                                   const Address &srcAddrs,
                                   const Address &dstAddrs,
                                   const SeqTsSizeHeader &seqTsSizeHeader)
{
  double delay = 0.0;
  std::ostringstream  oss;
  oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
      << "->"
      << Ipv4Address::ConvertFrom (localAddrs)
      << "("
      << seqTsSizeHeader.GetSeq ()
      << ")";
  std::string mapKey = oss.str ();
  auto it = g_rxPacketsForDelayCalc.find (mapKey);
  if (it == g_rxPacketsForDelayCalc.end ())
    {
      NS_FATAL_ERROR ("Rx packet not found?!");
    }
  else
    {
      delay = Simulator::Now ().GetSeconds () * 1000.0 - it->second.txTimestamp.GetSeconds () * 1000.0;
    }
  series->Append (Simulator::Now ().GetSeconds (), delay);
}
/*
 * Trace sink to calculate the packet delay upon reception at the
 * application layer and add it to a netSimulyzer EcdfSink.
 */
void
CdfTraceForDelayNetSimulyzer (Ptr<netsimulyzer::EcdfSink> ecdf,
                              Ptr<Node> node,
                              const Address &localAddrs,
                              Ptr<const Packet> p,
                              const Address &srcAddrs,
                              const Address &dstAddrs,
                              const SeqTsSizeHeader &seqTsSizeHeader)
{
  double delay = 0.0;
  std::ostringstream  oss;
  oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
      << "->"
      << Ipv4Address::ConvertFrom (localAddrs)
      << "("
      << seqTsSizeHeader.GetSeq ()
      << ")";
  std::string mapKey = oss.str ();
  auto it = g_rxPacketsForDelayCalc.find (mapKey);
  if (it == g_rxPacketsForDelayCalc.end ())
    {
      NS_FATAL_ERROR ("Rx packet not found?!");
    }
  else
    {
      delay = Simulator::Now ().GetSeconds () * 1000.0 - it->second.txTimestamp.GetSeconds () * 1000.0;
    }
  ecdf->Append (delay);
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
  //  double bandwidthBand = 400; //Multiple of 100 KHz; 400 = 40 MHz (This is from SL examples and does not work here. TODO: verify consistency of in-network, SL bandwidth config)
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
  uint16_t onOffPacketSize = 60; //bytes - MCPTT paper (AMR-WB) codec TODO
  double onOffDataRate = 24.0; //kb/s. - MCPTT paper (AMR-WB) codec TODO
  bool relayUesTraffic = true;


  //Simulation configuration
  std::string outputDir = "./";
  std::string exampleName = "nr-prose-relay-on-off";

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

  std::cout << "IP configuration: " << std::endl;
  std::cout << " Remote Host: " << remoteHostAddr << std::endl;


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

  nodeHelper.Set ("Model", netsimulyzer::models::SMARTPHONE_VALUE);
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{
    netsimulyzer::GREEN
  });
  for (uint32_t i = 0; i < inNetUeNodes.GetN (); ++i)
    {
      nodeHelper.Set ("Name", StringValue ("InNet UE - Node " + std::to_string (inNetUeNodes.Get (i)->GetId () )));
      nodeHelper.Install (inNetUeNodes.Get (i));
    }
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{
    netsimulyzer::BLUE
  });
  for (uint32_t i = 0; i < relayUeNodes.GetN (); ++i)
    {
      nodeHelper.Set ("Name", StringValue ("Relay UE - Node " + std::to_string (relayUeNodes.Get (i)->GetId () )));
      nodeHelper.Install (relayUeNodes.Get (i));
    }
  nodeHelper.Set ("HighlightColor", netsimulyzer::OptionalValue<netsimulyzer::Color3>{
    netsimulyzer::PURPLE
  });
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
  auto dlRxTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  dlRxTputCollection->SetAttribute ("Name", StringValue ("Throughput - All UEs - DL - Rx"));
  dlRxTputCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
  dlRxTputCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));

  auto ulRxTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  ulRxTputCollection->SetAttribute ("Name", StringValue ("Throughput - All UEs - UL - Rx"));
  ulRxTputCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
  ulRxTputCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));

  auto dlDelayEcdfInNet = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All InNet UEs - DL");
  dlDelayEcdfInNet->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  dlDelayEcdfInNet->GetSeries ()->SetAttribute ("Color", GetNextColor ());
  auto dlDelayEcdfRelays = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All Relay UEs - DL");
  dlDelayEcdfRelays->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  dlDelayEcdfRelays->GetSeries ()->SetAttribute ("Color", GetNextColor ());
  auto dlDelayEcdfRemotes = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All Remote UEs - DL");
  dlDelayEcdfRemotes->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  dlDelayEcdfRelays->GetSeries ()->SetAttribute ("Color", GetNextColor ());

  auto ulDelayEcdfInNet = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All InNet UEs - UL");
  ulDelayEcdfInNet->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  ulDelayEcdfInNet->GetSeries ()->SetAttribute ("Color", GetNextColor ());
  auto ulDelayEcdfRelays = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All Relay UEs - UL");
  ulDelayEcdfRelays->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  ulDelayEcdfRelays->GetSeries ()->SetAttribute ("Color", GetNextColor ());
  auto ulDelayEcdfRemotes = CreateObject<netsimulyzer::EcdfSink> (orchestrator, "Delay CDF - All Remote UEs - UL");
  ulDelayEcdfRemotes->GetXAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  ulDelayEcdfRemotes->GetSeries ()->SetAttribute ("Color", GetNextColor ());

  auto dlDelayEcdfCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  dlDelayEcdfCollection->SetAttribute ("Name", StringValue ("Delay CDF - All UEs - DL"));
  dlDelayEcdfCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  dlDelayEcdfCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Empirical CDF"));
  dlDelayEcdfCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));
  dlDelayEcdfCollection->Add (dlDelayEcdfInNet->GetSeries ());
  dlDelayEcdfCollection->Add (dlDelayEcdfRelays->GetSeries ());
  dlDelayEcdfCollection->Add (dlDelayEcdfRemotes->GetSeries ());

  auto ulDelayEcdfCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
  ulDelayEcdfCollection->SetAttribute ("Name", StringValue ("Delay CDF - All UEs - UL"));
  ulDelayEcdfCollection->GetAttribute ("XAxis", xAxis);
  xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
  ulDelayEcdfCollection->GetAttribute ("YAxis", yAxis);
  yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Empirical CDF"));
  ulDelayEcdfCollection->SetAttribute ("HideAddedSeries", BooleanValue (false));
  ulDelayEcdfCollection->Add (ulDelayEcdfInNet->GetSeries ());
  ulDelayEcdfCollection->Add (ulDelayEcdfRelays->GetSeries ());
  ulDelayEcdfCollection->Add (ulDelayEcdfRemotes->GetSeries ());


#endif


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

  //std::string onDistStr = "ns3::ExponentialRandomVariable[Mean=2.5]"; //3GPP 38.843 A.2.1.3
  //std::string onDistStr = "ns3::ExponentialRandomVariable[Mean=4.69]"; //MCPTT paper
  //std::string onDistStr = "ns3::ConstantRandomVariable[Constant=1.0]";
  std::string onDistStr = "ns3::ConstantRandomVariable[Constant=3.0]";

  //std::string offDistStr = "ns3::ExponentialRandomVariable[Mean=0.83]"; ////3GPP 38.843 A.2.1.3 - AF = 0.75 -> mean = (1/0.75 -1)*2.5
  //std::string offDistStr = "ns3::ExponentialRandomVariable[Mean=1.56]";  //MCPTT paper - VAF = 0.75 -> mean = (1/0.75 -1)*4.69
  //std::string offDistStr = "ns3::ConstantRandomVariable[Constant=0.0]";
  std::string offDistStr = "ns3::ConstantRandomVariable[Constant=2.0]";

  Config::SetDefault ("ns3::OnOffApplication::EnableSeqTsSizeHeader", BooleanValue (true));
  Config::SetDefault ("ns3::PacketSink::EnableSeqTsSizeHeader", BooleanValue (true));

  //InNetwork UEs
  std::cout << "InNetwork UEs traffic flows: " << std::endl;
  for (uint32_t inIdx = 0; inIdx < inNetUeNodes.GetN (); ++inIdx)
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
      std::cout << " DL: " << remoteHostAddr << " -> " << inNetIpv4AddressVector [inIdx] <<
        " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
      uint32_t nodeId = inNetUeNodes.Get (inIdx)->GetId ();

      auto dlUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      dlUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - InNet UE - DL - Node " + std::to_string (nodeId)));
      dlUeTputCollection->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      dlUeTputCollection->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
      auto dlTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "InNet UE - Tx - DL - Node " + std::to_string (nodeId));
      dlTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());

      dlClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlTxTput));
      dlUeTputCollection->Add (dlTxTput->GetSeries ());
#endif

      //-Server on UE
      PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (inNetUeNodes.Get (inIdx));
      serverApps.Add (dlSeverApp);

#ifdef HAS_NETSIMULYZER
      auto dlRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - InNet UE - Rx - DL - Node " + std::to_string (nodeId));
      dlRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      dlSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlRxTput));
      dlUeTputCollection->Add (dlRxTput->GetSeries ());
      dlRxTputCollection->Add (dlRxTput->GetSeries ());

      auto dlRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      dlRxDelay->SetAttribute ("Name", StringValue ("Delay - InNet UE - Rx - DL - Node " + std::to_string (nodeId)));
      dlRxDelay->SetAttribute ("Color", GetNextColor ());
      dlRxDelay->SetAttribute ("Connection", StringValue ("None"));
      dlRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      dlRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
      dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                         dlRxDelay,
                                                                         dlSeverApp.Get (0)->GetNode (),
                                                                         inNetIpv4AddressVector [inIdx]));
      dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                         dlDelayEcdfInNet,
                                                                         dlSeverApp.Get (0)->GetNode (),
                                                                         inNetIpv4AddressVector [inIdx]));
#endif

      //DL bearer configuration
      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      pfDl.localPortStart = dlPort; //TODO: Do I need it?
      pfDl.localPortEnd = dlPort; //TODO: Do I need it?
      ++dlPort;
      tftDl->Add (pfDl);
      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerDl, tftDl);

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
      std::cout << " UL: " << inNetIpv4AddressVector [inIdx] << " -> " << remoteHostAddr <<
        " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
      auto ulUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      ulUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - InNet UE - UL - Node " + std::to_string (nodeId)));
      ulUeTputCollection->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      ulUeTputCollection->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));

      auto ulTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - InNet UE - Tx - UL - Node " + std::to_string (nodeId));
      ulTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      ulClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulTxTput));
      ulUeTputCollection->Add (ulTxTput->GetSeries ());
#endif

      //-Server on Remtoe Host
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
      serverApps.Add (ulSeverApp);

#ifdef HAS_NETSIMULYZER
      auto ulRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - InNet UE - Rx - UL - Node " + std::to_string (nodeId));
      ulRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      ulSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulRxTput));
      ulUeTputCollection->Add (ulRxTput->GetSeries ());
      ulRxTputCollection->Add (ulRxTput->GetSeries ());

      auto ulRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      ulRxDelay->SetAttribute ("Name", StringValue ("Delay - InNet UE - Rx - UL - Node " + std::to_string (nodeId)));
      ulRxDelay->SetAttribute ("Color", GetNextColor ());
      ulRxDelay->SetAttribute ("Connection", StringValue ("None"));
      ulRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      ulRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
      ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                         ulRxDelay,
                                                                         ulSeverApp.Get (0)->GetNode (),
                                                                         remoteHostAddr));
      ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                         ulDelayEcdfInNet,
                                                                         ulSeverApp.Get (0)->GetNode (),
                                                                         remoteHostAddr));
#endif

      //UL bearer configuration
      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
      pfUl.remotePortStart = ulPort; //TODO: Do I need it?
      pfUl.remotePortEnd = ulPort; //TODO: Do I need it?
      ++ulPort;
      tftUl->Add (pfUl);
      enum EpsBearer::Qci qUl;
      qUl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerUl (qUl);
      nrHelper->ActivateDedicatedEpsBearer (inNetUeNetDev.Get (inIdx), bearerUl, tftUl);
    }

  //Remote UEs
  std::cout << "Remote UEs traffic flows: " << std::endl;
  for (uint32_t rmIdx = 0; rmIdx < remoteUeNodes.GetN (); ++rmIdx)
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
      std::cout << " DL: " << remoteHostAddr << " -> " << remotesIpv4AddressVector [rmIdx] <<
        " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
      uint32_t nodeId = remoteUeNodes.Get (rmIdx)->GetId ();

      auto dlUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      dlUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - Remote UE - DL - Node " + std::to_string (nodeId)));
      dlUeTputCollection->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      dlUeTputCollection->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
      auto dlTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Remote UE - Tx - DL - Node " + std::to_string (nodeId));
      dlTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      dlClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlTxTput));
      dlUeTputCollection->Add (dlTxTput->GetSeries ());
#endif

      //-Server on UE
      PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (remoteUeNodes.Get (rmIdx));
      serverApps.Add (dlSeverApp);

#ifdef HAS_NETSIMULYZER
      auto dlRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Remote UE - Rx - DL - Node " + std::to_string (nodeId));
      dlRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      dlSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlRxTput));
      dlUeTputCollection->Add (dlRxTput->GetSeries ());
      dlRxTputCollection->Add (dlRxTput->GetSeries ());

      auto dlRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      dlRxDelay->SetAttribute ("Name", StringValue ("Delay - Remote UE - Rx - DL - Node " + std::to_string (nodeId)));
      dlRxDelay->SetAttribute ("Color", GetNextColor ());
      dlRxDelay->SetAttribute ("Connection", StringValue ("None"));
      dlRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      dlRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
      dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                         dlRxDelay,
                                                                         dlSeverApp.Get (0)->GetNode (),
                                                                         remotesIpv4AddressVector [rmIdx]));
      dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                         dlDelayEcdfRemotes,
                                                                         dlSeverApp.Get (0)->GetNode (),
                                                                         remotesIpv4AddressVector [rmIdx]));
#endif

      //DL bearer configuration
      Ptr<EpcTft> tftDl = Create<EpcTft> ();
      EpcTft::PacketFilter pfDl;
      pfDl.localPortStart = dlPort; //TODO: Do I need it?
      pfDl.localPortEnd = dlPort; //TODO: Do I need it?
      ++dlPort;
      tftDl->Add (pfDl);
      enum EpsBearer::Qci qDl;
      qDl = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearerDl (qDl);
      nrHelper->ActivateDedicatedEpsBearer (remoteUeNetDev.Get (rmIdx), bearerDl, tftDl);

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
      std::cout << " UL: " << remotesIpv4AddressVector [rmIdx] << " -> " << remoteHostAddr <<
        " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
      auto ulUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
      ulUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - Remote UE - UL - Node " + std::to_string (nodeId)));
      ulUeTputCollection->GetAttribute ("XAxis", xAxis);
      xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
      ulUeTputCollection->GetAttribute ("YAxis", yAxis);
      yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));

      auto ulTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Remote UE - Tx - UL - Node " + std::to_string (nodeId));
      ulTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      ulClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulTxTput));
      ulUeTputCollection->Add (ulTxTput->GetSeries ());
#endif

      //-Server on Remote Host
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
      serverApps.Add (ulSeverApp);

#ifdef HAS_NETSIMULYZER
      auto ulRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Remote UE - Rx - UL - Node " + std::to_string (nodeId));
      ulRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
      ulSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulRxTput));
      ulUeTputCollection->Add (ulRxTput->GetSeries ());
      ulRxTputCollection->Add (ulRxTput->GetSeries ());

      auto ulRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
      ulRxDelay->SetAttribute ("Name", StringValue ("Delay - Remote UE - Rx - UL - Node " + std::to_string (nodeId)));
      ulRxDelay->SetAttribute ("Color", GetNextColor ());
      ulRxDelay->SetAttribute ("Connection", StringValue ("None"));
      ulRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
      ulRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
      ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                         ulRxDelay,
                                                                         ulSeverApp.Get (0)->GetNode (),
                                                                         remoteHostAddr));
      ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                      MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                         ulDelayEcdfRemotes,
                                                                         ulSeverApp.Get (0)->GetNode (),
                                                                         remoteHostAddr));
#endif

      //UL bearer configuration
      Ptr<EpcTft> tftUl = Create<EpcTft> ();
      EpcTft::PacketFilter pfUl;
      pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
      pfUl.remotePortStart = ulPort; //TODO: Do I need it?
      pfUl.remotePortEnd = ulPort; //TODO: Do I need it?
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
      std::cout << "Relay UEs traffic flows: " << std::endl;
      for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
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
          std::cout << " DL: " << remoteHostAddr << " -> " << relaysIpv4AddressVector [ryIdx] <<
            " start time: " << dlAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
          uint32_t nodeId = relayUeNodes.Get (ryIdx)->GetId ();

          auto dlUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
          dlUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - Relay UE - DL - Node " + std::to_string (nodeId)));
          dlUeTputCollection->GetAttribute ("XAxis", xAxis);
          xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
          dlUeTputCollection->GetAttribute ("YAxis", yAxis);
          yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
          auto dlTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Relay UE - Tx - DL - Node " + std::to_string (nodeId));
          dlTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
          dlClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlTxTput));
          dlUeTputCollection->Add (dlTxTput->GetSeries ());
#endif

          //-Server in UE
          PacketSinkHelper dlpacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          ApplicationContainer dlSeverApp = dlpacketSinkHelper.Install (relayUeNodes.Get (ryIdx));
          serverApps.Add (dlSeverApp);

#ifdef HAS_NETSIMULYZER
          auto dlRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Relay UE - Rx - DL - Node " + std::to_string (nodeId));
          dlRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
          dlSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, dlRxTput));
          dlUeTputCollection->Add (dlRxTput->GetSeries ());
          dlRxTputCollection->Add (dlRxTput->GetSeries ());

          auto dlRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
          dlRxDelay->SetAttribute ("Name", StringValue ("Delay - Relay UE - Rx - DL - Node " + std::to_string (nodeId)));
          dlRxDelay->SetAttribute ("Color", GetNextColor ());
          dlRxDelay->SetAttribute ("Connection", StringValue ("None"));
          dlRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
          dlRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
          dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                          MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                             dlRxDelay,
                                                                             dlSeverApp.Get (0)->GetNode (),
                                                                             relaysIpv4AddressVector [ryIdx]));
          dlSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                          MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                             dlDelayEcdfRelays,
                                                                             dlSeverApp.Get (0)->GetNode (),
                                                                             relaysIpv4AddressVector [ryIdx]));
#endif

          //DL bearer configuration
          Ptr<EpcTft> tftDl = Create<EpcTft> ();
          EpcTft::PacketFilter pfDl;
          pfDl.localPortStart = dlPort; //TODO: Do I need it?
          pfDl.localPortEnd = dlPort; //TODO: Do I need it?
          ++dlPort;
          tftDl->Add (pfDl);
          enum EpsBearer::Qci qDl;
          qDl = EpsBearer::GBR_CONV_VOICE;
          EpsBearer bearerDl (qDl);
          nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerDl, tftDl);

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
          std::cout << " UL: " << relaysIpv4AddressVector [ryIdx] << " -> " << remoteHostAddr  <<
            " start time: " << ulAppStartTime.GetSeconds ()  << " s, end time: " << simTime.GetSeconds () << " s" << std::endl;

#ifdef HAS_NETSIMULYZER
          auto ulUeTputCollection = CreateObject<netsimulyzer::SeriesCollection> (orchestrator);
          ulUeTputCollection->SetAttribute ("Name", StringValue ("Throughput - Relay UE - UL - Node " + std::to_string (nodeId)));
          ulUeTputCollection->GetAttribute ("XAxis", xAxis);
          xAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Time (s)"));
          ulUeTputCollection->GetAttribute ("YAxis", yAxis);
          yAxis.Get<netsimulyzer::ValueAxis> ()->SetAttribute ("Name", StringValue ("Throughput (kb/s)"));
          auto ulTxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Relay UE - Tx - UL - Node " + std::to_string (nodeId));
          ulTxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
          ulClientApp.Get (0)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulTxTput));
          ulUeTputCollection->Add (ulTxTput->GetSeries ());
#endif


          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          ApplicationContainer ulSeverApp = ulPacketSinkHelper.Install (remoteHost);
          serverApps.Add (ulSeverApp);

#ifdef HAS_NETSIMULYZER
          auto ulRxTput = CreateObject<netsimulyzer::ThroughputSink> (orchestrator, "Throughput - Relay UE - Rx - UL - Node " + std::to_string (nodeId));
          ulRxTput->GetSeries ()->SetAttribute ("Color", GetNextColor ());
          ulSeverApp.Get (0)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&PacketTraceNetSimulyzer, ulRxTput));
          ulUeTputCollection->Add (ulRxTput->GetSeries ());
          ulRxTputCollection->Add (ulRxTput->GetSeries ());

          auto ulRxDelay = CreateObject <netsimulyzer::XYSeries> (orchestrator);
          ulRxDelay->SetAttribute ("Name", StringValue ("Delay - Relay UE - Rx - UL - Node " + std::to_string (nodeId)));
          ulRxDelay->SetAttribute ("Color", GetNextColor ());
          ulRxDelay->SetAttribute ("Connection", StringValue ("None"));
          ulRxDelay->GetXAxis ()->SetAttribute ("Name", StringValue ("Time (s)"));
          ulRxDelay->GetYAxis ()->SetAttribute ("Name", StringValue ("Packet delay (ms)"));
          ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                          MakeBoundCallback (&RxPacketTraceForDelayNetSimulyzer,
                                                                             ulRxDelay,
                                                                             ulSeverApp.Get (0)->GetNode (),
                                                                             remoteHostAddr));
          ulSeverApp.Get (0)->TraceConnectWithoutContext ("RxWithSeqTsSize",
                                                          MakeBoundCallback (&CdfTraceForDelayNetSimulyzer,
                                                                             ulDelayEcdfRelays,
                                                                             ulSeverApp.Get (0)->GetNode (),
                                                                             remoteHostAddr));
#endif

          //UL bearer configuration
          Ptr<EpcTft> tftUl = Create<EpcTft> ();
          EpcTft::PacketFilter pfUl;
          pfUl.remoteAddress = remoteHostAddr; //IMPORTANT!!!
          pfUl.remotePortStart = ulPort; //TODO: Do I need it?
          pfUl.remotePortEnd = ulPort; //TODO: Do I need it?
          ++ulPort;
          tftUl->Add (pfUl);
          enum EpsBearer::Qci qUl;
          qUl = EpsBearer::GBR_CONV_VOICE;
          EpsBearer bearerUl (qUl);
          nrHelper->ActivateDedicatedEpsBearer (relayUeNetDev.Get (ryIdx), bearerUl, tftUl);
        }

    }

  clientApps.Stop (simTime);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (simTime);
  /******************** End Application configuration ************************/


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

  UeToUePktTxRxOutputStats pktStats;
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


  /******************* Application packet delay tracing ********************************/
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> PacketTraceForDelayStream = ascii.CreateFileStream ("NrSlAppRxPacketDelayTrace.txt");
  *PacketTraceForDelayStream->GetStream () << "time(s)\trxNodeId\tsrcIp\tdstIp\tseqNum\tdelay(ms)" << std::endl;

  for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      clientApps.Get (ac)->TraceConnectWithoutContext ("TxWithSeqTsSize", MakeBoundCallback (&TxPacketTraceForDelay, localAddrs));
    }
  for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      serverApps.Get (ac)->TraceConnectWithoutContext ("RxWithSeqTsSize", MakeBoundCallback (&RxPacketTraceForDelay, PacketTraceForDelayStream, serverApps.Get (ac)->GetNode (), localAddrs));
    }
  /******************* END Application packet delay tracing ****************************/

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

  /*
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();
  outputConfig.ConfigureDefaults ();
   */
  Simulator::Run ();

  //SL database dump
  pktStats.EmptyCache ();
  pscchStats.EmptyCache ();
  psschStats.EmptyCache ();
  pscchPhyStats.EmptyCache ();
  psschPhyStats.EmptyCache ();
  ueRlcRxStats.EmptyCache ();

  //Print per-flow statistics
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

      appDuration = simTime.GetSeconds () - timeStartTraffic.GetSeconds (); //TODO Some inaccuracy is expected due to randomization of start time.

      outFile << "  Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ") " << protoStream.str () << "\n";
      outFile << "    Tx Packets: " << i->second.txPackets << "\n";
      outFile << "    Tx Bytes:   " << i->second.txBytes << "\n";
      outFile << "    TxOffered:  " << i->second.txBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps\n";
      outFile << "    Rx Packets: " << i->second.rxPackets << "\n";
      outFile << "    Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          outFile << "    Throughput: " << i->second.rxBytes * 8.0 / appDuration / 1000 / 1000  << " Mbps\n";
          outFile << "    Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          outFile << "    Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          outFile << "    Throughput:  0 Mbps\n";
          outFile << "    Mean delay:  0 ms\n";
          outFile << "    Mean jitter: 0 ms\n";
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


