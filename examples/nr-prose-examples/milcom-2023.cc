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


uint32_t g_nConnectedRemoteUes {0};  //!< Global variable to count the number of succesful remote UE conections to a relay UE

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
  if (!isTx && pc5smt.GetMessageName ()  == "PROSE DIRECT LINK ESTABLISHMENT ACCEPT")
  {
    //Succesfull connection
    g_nConnectedRemoteUes ++; 
  }

  if (stream != nullptr)
    {
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


void
TraceSensingAlgorithm (const struct NrUeMac::SensingTraceReport& report,
    const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& candidates,
    const std::list<SensingData>& sensingData, const std::list<SfnSf>& transmitHistory) 
{
/*   NS_LOG_DEBUG ("Sfn " << report.m_sfn.Normalize () 
                 << " nSlots " << report.m_initialCandidateSlotsSize 
                 << " nSubCh " << report.m_subchannels
                 << " sensing info size " << sensingData.size () 
                 << " history info size " << transmitHistory.size ());
  NS_LOG_DEBUG ("-> nResources:" 
                 << " initial " << report.m_initialCandidateResourcesSize 
                 << " afterS5 " << report.m_candidateResourcesSizeAfterStep5
                 << " final " << candidates.size ()  
);
*/

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
 * Structure to store the application layer packet history of a traffic flow.
 */
struct TrafficFlowHistory
{
  double nTxPackets {0};  //!< Number of transmitted packets
  double nRxPackets {0};  //!< Number of received packets
  double sumDelay {0};    //!< Sum of the packet delays (used to calculate average packet delay at the end of simulation)
  std::map<uint32_t, Time> txPacketsHistory; //!< Transmited packets history. The key is the packet sequence number, the value is the time stamp of the transmission of the packet
  uint32_t rxNodeId {0};
  std::string direction {"-"};
  Time lastPacketRxTimestamp {Seconds(0)};
  double sumJitter {0};    //!< Sum of the packet jitter (used to calculate average jitter at the end of simulation)

};

/*
 * Map to store the application layer packet history per traffic flow.
 * The key is the string srcAddr->dstAddr of the traffic flow
 */
std::map<std::string, TrafficFlowHistory> g_trafficFlowsHistory;


/*
 * Map to store received packets and reception timestamps at the application
 * layer. Used to calculate packet delay at the application layer.
 */
std::map<std::string, PacketWithRxTimestamp> g_packetsForDelayCalc;
uint32_t g_nTxPackets {0}; //!< Global variable to count TX packets 
uint32_t g_nRxPackets {0}; //!< Global variable to count RX packets
std::list <double> g_delays; //!< Global list to store packet delays upon RX
uint32_t g_nTxPackets_UL {0}; //!< Global variable to count TX packets of flows in the uplink direction
uint32_t g_nRxPackets_UL {0}; //!< Global variable to count RX packets of flows in the uplink direction
std::list <double> g_delays_UL; //!< Global list to store packet delays upon RX of flows in the uplink direction
uint32_t g_nTxPackets_DL {0}; //!< Global variable to count TX packets of flows in the uplink direction
uint32_t g_nRxPackets_DL {0}; //!< Global variable to count RX packets of flows in the uplink direction
std::list <double> g_delays_DL; //!< Global list to store packet delays upon RX of flows in the uplink direction


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
TxPacketTrace (Ptr<Node> node, const Address &localAddrs, Ptr<const Packet> p, const Address &srcAddrs,
               const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  g_nTxPackets ++;
  if (node->GetDevice(0)->GetObject<NrUeNetDevice> () == nullptr)
    {
      //It is the remote host who is transmitting the packet -> DL
      g_nTxPackets_DL ++;
    }
  else
    {
      //It is the UE who is transmitting the packet -> UL
      g_nTxPackets_UL ++;
    }
  //std::cout << "TX " << Simulator::Now ().GetSeconds () * 1000.0 <<std::endl;

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

  std::ostringstream  oss1;
  //Create key
  oss1 << Ipv4Address::ConvertFrom (localAddrs)
  << "->"
  << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
  std::string trafficHistorymapKey = oss1.str ();

  //std::cout << trafficHistorymapKey <<std::endl;
  auto it = g_trafficFlowsHistory.find (trafficHistorymapKey);
  if (it == g_trafficFlowsHistory.end ())
    {
      //1st packet
      TrafficFlowHistory trafficFlowHistory;
      trafficFlowHistory.nTxPackets += 1;
      trafficFlowHistory.txPacketsHistory.insert(std::pair <uint32_t, Time> (seqTsSizeHeader.GetSeq (), Simulator::Now ()));
      g_trafficFlowsHistory.insert (std::pair <std::string, TrafficFlowHistory> (trafficHistorymapKey, trafficFlowHistory));
    }
  else
    {
      //Other packets
      it->second.nTxPackets += 1;
      it->second.txPacketsHistory.insert(std::pair <uint32_t, Time> (seqTsSizeHeader.GetSeq (), Simulator::Now ()));
    }
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
   if (node->GetDevice(0)->GetObject<NrUeNetDevice> () == nullptr)
    {
      //It is the remote host who received the packet -> UL
      g_nRxPackets_UL ++;
    }
  else
    {
      //It is the UE who received the packet -> DL
      g_nRxPackets_DL ++;
    }
  double delay = 0.0;
  double jitter = 0.0;
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
      if (node->GetDevice(0)->GetObject<NrUeNetDevice> () == nullptr)
        {
          //It is the remote host who received the packet -> UL
          g_delays_UL.push_back (delay);
        }
      else
        {
          //It is the UE who received the packet -> DL
          g_delays_DL.push_back (delay);
        }
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

  std::ostringstream  oss1;
  oss1 << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ()
                      << "->"
                      << Ipv4Address::ConvertFrom (localAddrs);
  std::string trafficHistorymapKey = oss1.str ();
  auto itTrafficHistory = g_trafficFlowsHistory.find (trafficHistorymapKey);
  if (itTrafficHistory == g_trafficFlowsHistory.end ())
    {
      NS_FATAL_ERROR ("Traffic flow not found?!");
    }
  else
    {
      if (itTrafficHistory->second.rxNodeId == 0)
        {
          //First reception, initialize values
          itTrafficHistory->second.rxNodeId = node->GetId ();
          if (node->GetDevice(0)->GetObject<NrUeNetDevice> () == nullptr)
            {
              //It is the remote host who received the packet
              itTrafficHistory->second.direction = "UL" ;
            }
          else
            {
              //It is the UE who received the packet
              itTrafficHistory->second.direction = "DL";
            }
        }

      auto itPacketTimestamp = itTrafficHistory->second.txPacketsHistory.find (seqTsSizeHeader.GetSeq ());
      if (itPacketTimestamp == itTrafficHistory->second.txPacketsHistory.end ())

        {
          NS_FATAL_ERROR ("Rx packet not found?!");
        }
      else
        {

          delay = Simulator::Now ().GetSeconds () * 1000.0 - itPacketTimestamp->second.GetSeconds () * 1000.0;
          itTrafficHistory->second.txPacketsHistory.erase (seqTsSizeHeader.GetSeq ());
          itTrafficHistory->second.sumDelay += delay;
          itTrafficHistory->second.nRxPackets += 1;
 //         std::cout << trafficHistorymapKey << " " << seqTsSizeHeader.GetSeq ()
   //           << " " << delay << " " << itTrafficHistory->second.sumDelay << std::endl;

          //Caculate jitter 
          if (itTrafficHistory->second.lastPacketRxTimestamp != Seconds(0))
          {
            //Not the first reception
            jitter = Simulator::Now ().GetSeconds () * 1000.0 - itTrafficHistory->second.lastPacketRxTimestamp.GetSeconds () * 1000.0;
            itTrafficHistory->second.sumJitter += jitter;
            itTrafficHistory->second.lastPacketRxTimestamp = Simulator::Now ();
//            std::cout << trafficHistorymapKey << " " << seqTsSizeHeader.GetSeq ()
//              << " " << jitter << " " << itTrafficHistory->second.sumJitter << std::endl;
          }
          else
          {
            //First reception
            itTrafficHistory->second.lastPacketRxTimestamp = Simulator::Now ();
          }
        }
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
 * \param platoonRadius
 * \param squadRadius
 */
void


GenerateTopologyPlotFile (std::string fileNameWithNoExtension, NodeContainer gNbNode,
                          NodeContainer vehicleUeNodes, NodeContainer soldierUeNodes,
                          double platoonRadius, double squadRadius)
{
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string gnuplotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "Topology (Labels = Node IDs)";

  Gnuplot plot (graphicsFileName);
  plot.SetTitle (plotTitle);
  plot.SetTerminal ("png size 1024,1024");
  plot.SetLegend ("X (m)", "Y (m)"); //These are the axis, not the legend
  std::ostringstream plotExtras;
  plotExtras << "set xrange [-" << 1.1 * platoonRadius + squadRadius << ":+" << 1.1 * platoonRadius + squadRadius << "]" << std::endl;
  plotExtras << "set yrange [-" << 1.1 * platoonRadius + squadRadius << ":+" << 1.1 * platoonRadius + squadRadius << "]" << std::endl;
  plotExtras << "set linetype 1 pt 3 ps 2 " << std::endl;
  plotExtras << "set linetype 2 lc rgb \"green\" pt 2 ps 2" << std::endl;
  plotExtras << "set linetype 3 pt 1 ps 2" << std::endl;
  plotExtras << "set obj circle at "<< 0 << "," << 0 << " size " << platoonRadius
             << " fs empty border rgb \"yellow\"" << std::endl;


  //Create squad areas
  for (uint32_t ryIdx = 0; ryIdx < vehicleUeNodes.GetN (); ryIdx++)
    {
      double x = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().x;
      double y = vehicleUeNodes.Get (ryIdx)->GetObject<MobilityModel> ()->GetPosition ().y;
      plotExtras << "set obj circle at "<< x << "," << y << " size " << squadRadius
                 << " fs empty border rgb \"green\"" << std::endl;

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
  double centralFrequencyBand = 5.89e9; // band n47 (From SL examples)
  //double centralFrequencyBand = 795e6; // 795 MHz band n14
  double bandwidthBand = 80e6; //40 MHz
  double centralFrequencyCc0 = 5.89e9;
  //double centralFrequencyCc0 = 795e6; // 795 MHz band n14
  double bandwidthCc0 = bandwidthBand;
  std::string pattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|";
  double bandwidthCc0Bpw0 = bandwidthCc0 / 2;
  double bandwidthCc0Bpw1 = bandwidthCc0 / 2;
  double ueTxPower = 23; //dBm

  //In-network devices configuration
  uint16_t numerologyCc0Bwp0 = 2; // BWP0 will be used for the in-network
  //uint16_t numerologyCc0Bwp0 = 0; // BWP0 will be used for the in-network

  double gNBtotalTxPower = 32; // dBm
  bool cellScan = false;  // Beamforming method.
  double beamSearchAngleStep = 10.0; // Beamforming parameter.

  //Sidelink configuration
  uint16_t numerologyCc0Bwp1 = 2; // BWP1 will be used for SL TODO: things that depend on this: t2, SPS RRI
  Time startRelayConnTime = Seconds (2.0); //Time to start the U2N relay connection establishment

  //Topology
  double gNbHeight = 10; //meters
  double vehicleUeHeight = 1.5; //meters
  double soldierUeHeight = 1.5; //meters
  uint16_t nSquads = 1;
  uint16_t nSoldiersPerSquad = 9;
  double platoonRadius = 50.0; //meters
  double squadRadius = 20.0; //meters
  bool soldierAttachToSquadVehicle = true;

  Time timeStartTraffic = Seconds (5.0);
  Time durationTraffic = Seconds (10.0);
  uint16_t onOffPacketSize = 60; //bytes
  double onOffDataRate = 24.0; //kbps
  bool ulTraffic = true;
  bool dlTraffic = true;
  bool relayUesDlTraffic = false;
  bool relayUesUlTraffic = false;
  bool inNetTraffic = false;
  double ulPercentage = 100.0;
  double dlPercentage = 100.0;

  //SL scheduling configuration
  bool enableSensing = false;
  uint16_t rri = 0; //MS
  uint16_t maxNumTx = 5; 
  uint16_t mcs = 5;


  //Simulation configuration
  std::string exampleName = "milcom-2023";
  bool writeTraces = false;

  CommandLine cmd;
  cmd.AddValue ("nSquads", "", nSquads);
  cmd.AddValue ("nSoldiersPerSquad", "", nSoldiersPerSquad);
  cmd.AddValue ("platoonRadius", "", platoonRadius);
  cmd.AddValue ("squadRadius", "", squadRadius);
  cmd.AddValue ("durationTraffic", " Seconds ", durationTraffic);
  cmd.AddValue ("onOffPacketSize", "", onOffPacketSize);
  cmd.AddValue ("onOffDataRate", "", onOffDataRate);
  cmd.AddValue ("ulTraffic", "", ulTraffic);
  cmd.AddValue ("dlTraffic", "", dlTraffic);
  cmd.AddValue ("relayUesUlTraffic", "", relayUesUlTraffic);
  cmd.AddValue ("relayUesDlTraffic", "", relayUesDlTraffic);
  cmd.AddValue ("enableSensing", "True if sensing is activated", enableSensing);
  cmd.AddValue ("numerologySl", "Numerology to be used for the sidelink", numerologyCc0Bwp1);
  cmd.AddValue ("rri", "Milliseconds", rri);
  cmd.AddValue ("maxNumTx", "", maxNumTx);
  cmd.AddValue ("ulPercentage", "", ulPercentage);
  cmd.AddValue ("dlPercentage", "", dlPercentage);
  cmd.AddValue ("mcs", "", mcs);
  cmd.AddValue ("writeTraces", "", writeTraces);



  cmd.Parse (argc, argv);

  NS_LOG_DEBUG ("Numerology SL " << numerologyCc0Bwp1 );
  NS_LOG_DEBUG ("Maximum number of transmissions " << maxNumTx );


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
  platoonRndVar->SetAttribute ("Min", DoubleValue (-platoonRadius));
  platoonRndVar->SetAttribute ("Max", DoubleValue (platoonRadius)); //meters

  Ptr<UniformRandomVariable> squadRndVar = CreateObject<UniformRandomVariable> ();
  squadRndVar->SetAttribute ("Min", DoubleValue (-squadRadius));
  squadRndVar->SetAttribute ("Max", DoubleValue (squadRadius)); //meters

  vehicleUeNodes.Create (nSquads);
  soldierUeNodes.Create (nSquads * nSoldiersPerSquad);

  //Position squads
  Ptr<ListPositionAllocator> posAllocVehicles = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> posAllocSoldiers = CreateObject<ListPositionAllocator> ();

  //Vehicle allocated randomly within the platoonRadius
  for (uint32_t vehicleIdx = 0; vehicleIdx < nSquads; ++vehicleIdx)
    {
      double posVehicle_x = platoonRndVar->GetValue();
      double posVehicle_y = platoonRndVar->GetValue();

      while ((posVehicle_x - 0.0) * (posVehicle_x - 0.0) + (posVehicle_y - 0.0) * (posVehicle_y - 0.0)  > platoonRadius * platoonRadius)
        {
          posVehicle_x = platoonRndVar->GetValue();
          posVehicle_y = platoonRndVar->GetValue();
        }

      posAllocVehicles->Add (Vector (posVehicle_x, posVehicle_y, vehicleUeHeight));

      //Soldiers allocated randomly within the squadRadius
      for (uint32_t soldierIdx = 0; soldierIdx < nSoldiersPerSquad; ++soldierIdx)
        {
          double posSoldier_x = posVehicle_x + squadRndVar->GetValue();
          double posSoldier_y = posVehicle_y + squadRndVar->GetValue();
          while ((posSoldier_x - posVehicle_x) * (posSoldier_x - posVehicle_x) + (posSoldier_y - posVehicle_y) * (posSoldier_y - posVehicle_y)  > squadRadius * squadRadius)
            {
              posSoldier_x = posVehicle_x + squadRndVar->GetValue();
              posSoldier_y = posVehicle_y + squadRndVar->GetValue();
            }
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
                                platoonRadius, squadRadius);
    }

  //All soldiers UEs are OoC and remote UEs
  for (uint32_t ueIdx = 0; ueIdx < soldierUeNodes.GetN (); ueIdx++)
    {
      remoteUeNodes.Add(soldierUeNodes.Get (ueIdx));
    }


  //All vehicles are relays
  for (uint32_t ueIdx = 0; ueIdx < vehicleUeNodes.GetN (); ueIdx++)
    {
      relayUeNodes.Add(vehicleUeNodes.Get (ueIdx));
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
  bwp1->m_scenario = BandwidthPartInfo::RMa_LoS;

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

  //Resource selection window length (given by T1 and T2) depend on the numerology
  //TODO: SPS RRI should be selected larger than selection window
  uint16_t t2;
  switch (numerologyCc0Bwp1)
  {
    case 0:
      //t2 = 33; // with T1 = 2, this gives a window of 32 slots with mu = 0 => 32 ms
      t2 = 17; // with T1 = 2, this gives a window of 16 slots with mu = 0 => 16 ms

      break;
    case 1:
      t2 = 33; // with T1 = 2, this gives a window of 32 slots with mu = 1 => 16 ms

      break;
    case 2:
      //t2 = 33; // with T1 = 2, this gives a window of 32 slots with mu = 2 => 8 ms
      t2 = 65; // with T1 = 2, this gives a window of 64 slots with mu = 2 => 16 ms
      break;

    default:
      NS_FATAL_ERROR ("Numerology for sidelink not recognized");
      break;
  }
  NS_LOG_DEBUG ("T2: " << t2 );

  nrHelper->SetUeMacAttribute ("EnableSensing", BooleanValue (enableSensing));
  nrHelper->SetUeMacAttribute ("T1", UintegerValue (2));
  nrHelper->SetUeMacAttribute ("T2", UintegerValue (t2));
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
  nrSlHelper->SetUeSlSchedulerAttribute ("InitialNrSlMcs", UintegerValue (mcs));

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
  std::vector <std::bitset<1> > slBitmap = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ptrFactory->SetSlTimeResources (slBitmap);
  ptrFactory->SetSlSensingWindow (100); // T0 in ms
  ptrFactory->SetSlSelectionWindow (5);
  ptrFactory->SetSlFreqResourcePscch (10); // PSCCH RBs
  ptrFactory->SetSlSubchannelSize (10);
  ptrFactory->SetSlMaxNumPerReserve (3);
  std::list<uint16_t> resourceReservePeriodList = {0, rri}; // in ms
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
  psschParams.slMaxTxTransNumPssch = maxNumTx;
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
  Config::SetDefault ("ns3::NrSlUeProseDirectLink::PdlEsRqRtxMax", UintegerValue (6));

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
  remoteUeSlInfo.m_harqEnabled = false;
  remoteUeSlInfo.m_priority = 0;

  SidelinkInfo relayUeSlInfo;
  relayUeSlInfo.m_castType = SidelinkInfo::CastType::Unicast;
  relayUeSlInfo.m_harqEnabled = false;
  relayUeSlInfo.m_priority = 0;

  if (rri > 0 )
  {
    //SPS
    NS_LOG_INFO ("SPS scheduling, RRI " << rri);
    remoteUeSlInfo.m_dynamic = false;
    relayUeSlInfo.m_dynamic = false;
    remoteUeSlInfo.m_rri = MilliSeconds (rri);
    relayUeSlInfo.m_rri = MilliSeconds (rri);
  }
  else
  {
    //Dynamic
    NS_LOG_INFO ("Dynamic scheduling" );
    remoteUeSlInfo.m_dynamic = true;
    relayUeSlInfo.m_dynamic = true;
    remoteUeSlInfo.m_rri = MilliSeconds (0);
    relayUeSlInfo.m_rri = MilliSeconds (0);
  }

  if (soldierAttachToSquadVehicle)
    {
      //Option 1: Each OOC soldier connects to the vehicle in their squad if in coverage
      //This logic works only if all soldiers are OOC
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

  std::string onDistStr = "ns3::ConstantRandomVariable[Constant=" + std::to_string (durationTraffic.GetSeconds ()) + "]";
//  std::string onDistStr = "ns3::ConstantRandomVariable[Constant=1.0]";
  std::string offDistStr = "ns3::ConstantRandomVariable[Constant=0.0]";
//  std::string offDistStr = "ns3::ConstantRandomVariable[Constant=1.0]";

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
          dataRateString  = std::to_string (onOffDataRate * (dlPercentage / 100.0)) + "kb/s";

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
          dataRateString  = std::to_string (onOffDataRate * (ulPercentage / 100.0)) + "kb/s";

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
 //      dataRateString  = std::to_string (9*onOffDataRate) + "kb/s"; //TODO: Remove, just a test
      dataRateString  = std::to_string (onOffDataRate) + "kb/s";

      NS_LOG_DEBUG ("Relay UEs traffic flows: ");
      for (uint32_t ryIdx = 0; ryIdx < relayUeNodes.GetN (); ++ryIdx)
        {
         if (relayUesDlTraffic)
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
          if (relayUesUlTraffic)
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
      clientApps.Get (ac)->TraceConnectWithoutContext ("TxWithSeqTsSize", MakeBoundCallback (&TxPacketTrace, clientApps.Get (ac)->GetNode (), localAddrs));
    }
  for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
    {
      Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
      serverApps.Get (ac)->TraceConnectWithoutContext ("RxWithSeqTsSize", MakeBoundCallback (&RxPacketTrace, PacketTraceForDelayStream, serverApps.Get (ac)->GetNode (), localAddrs));
    }
  /******************* END Application packet delay tracing ****************************/

  /******************* PC5-S messages tracing ********************************/
  Ptr<OutputStreamWrapper> Pc5SignallingPacketTraceStream;
  if (writeTraces)
    {
      Pc5SignallingPacketTraceStream= ascii.CreateFileStream ("NrSlPc5SignallingPacketTrace.txt");
      *Pc5SignallingPacketTraceStream->GetStream () << "time(s)\tnodeId\tTX/RX\tsrcL2Id\tdstL2Id\tmsgType" << std::endl;
    }
    else
    {
      Pc5SignallingPacketTraceStream = nullptr;
    }
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
  if (writeTraces)
    {
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

    Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SensingAlgorithm",
                                MakeCallback (&TraceSensingAlgorithm));





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
  NS_LOG_DEBUG ("System Packet delivery ratio " <<  ((double)g_nRxPackets / g_nTxPackets));
  NS_LOG_DEBUG ("System Average packet delay = " << delaySum / g_delays.size () << " ms");
  double delaySum_UL = 0;
  for (auto it = g_delays_UL.begin () ; it != g_delays_UL.end (); it ++ )
    {
      delaySum_UL += *it;
    }
  NS_LOG_DEBUG ("UL Packet delivery ratio " <<  ((double)g_nRxPackets_UL / g_nTxPackets_UL));
  NS_LOG_DEBUG ("UL Average packet delay = " << delaySum_UL / g_delays_UL.size () << " ms");
  double delaySum_DL = 0;
  for (auto it = g_delays_DL.begin () ; it != g_delays_DL.end (); it ++ )
    {
      delaySum_DL += *it;
    }
  NS_LOG_DEBUG ("DL Packet delivery ratio " <<  ((double)g_nRxPackets_DL / g_nTxPackets_DL));
  NS_LOG_DEBUG ("DL Average packet delay = " << delaySum_DL / g_delays_DL.size () << " ms");
 
  double sysJitterSum_UL = 0;
  double sysJitterSum_DL = 0;
  double nULflows = 0;
  double nDLflows = 0;

  //Per traffic flow stats:
  std::ostringstream  ossPerFlow;
  ossPerFlow << "srcAddr->dstAddr\trxNodeId\tdirection\tPDR\tavgDelay(ms)\tavgJitter(ms)" << std::endl;
  for (auto it = g_trafficFlowsHistory.begin () ; it != g_trafficFlowsHistory.end (); it ++ )
    {
      double averageDelay = it->second.sumDelay / it->second.nRxPackets;
      double pdr = it->second.nRxPackets / it->second.nTxPackets;
      double averageJitter = it->second.sumJitter / it->second.nRxPackets;
      ossPerFlow << it->first  << "\t" << it->second.rxNodeId  << "\t" << it->second.direction << "\t" 
                 <<  pdr << "\t" <<  averageDelay << "\t" << averageJitter <<std::endl;
      
      if (it->second.direction == "UL")
      {
        sysJitterSum_UL += averageJitter;
        nULflows ++;
      }
      if (it->second.direction == "DL")
      {
        sysJitterSum_DL += averageJitter;
        nDLflows ++;
      }      
    }
  //Standard output
  std::cout<< ossPerFlow.str ();

  //To file
  std::ofstream outFile;
  std::string filename = exampleName  + "-stats_perFlow.txt";
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << ossPerFlow.str ();
  outFile.close ();

  //System stats:
  std::ostringstream  ossSys;
  ossSys << "SysPDR\tSysAvgPacketDelay(ms)\tSysAvgFlowJitter(ms)\tratioConn\tUlPDR\tUlAvgPacketDelay(ms)\tUlAvgFlowJitter(ms)\tDlPDR\tDlAvgPacketDelay(ms)\tDlAvgFlowJitter(ms)" << std::endl;
  ossSys << ((double)g_nRxPackets / g_nTxPackets) << "\t" << delaySum / g_delays.size () << "\t" 
         << (sysJitterSum_UL + sysJitterSum_DL) / (nULflows + nDLflows) << "\t" 
         << ((double)g_nConnectedRemoteUes / (nSquads * nSoldiersPerSquad )) << "\t"
         << ((double)g_nRxPackets_UL / g_nTxPackets_UL) << "\t" << delaySum_UL / g_delays_UL.size () << "\t"
         << sysJitterSum_UL / nULflows << "\t" 
         << ((double)g_nRxPackets_DL / g_nTxPackets_DL) << "\t" << delaySum_DL / g_delays_DL.size () << "\t"
         << sysJitterSum_DL / nDLflows  
         << std::endl;
  //Standard output
  std::cout<< ossSys.str ();

  //To file
  filename = exampleName  + "-stats_system.txt";
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << ossSys.str ();
  outFile.close ();


  

  Simulator::Destroy ();
  return 0;
}


