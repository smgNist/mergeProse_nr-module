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


#include "nr-sl-ue-prose.h"

#include <ns3/lte-ue-rrc.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/lte-radio-bearer-info.h>

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

#include <algorithm>

#include <ns3/nr-sl-pc5-signalling-header.h>
#include <ns3/nr-point-to-point-epc-helper.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeProse");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeProse);


NrSlUeProseDirLinkContext::NrSlUeProseDirLinkContext (void)
{
  NS_LOG_FUNCTION (this);
  m_hasActiveSlDrb = false;
  m_hasPendingSlDrb = false;
  m_relayServiceCode = 0;
}

NrSlUeProseDirLinkContext::~NrSlUeProseDirLinkContext (void)
{}

TypeId
NrSlUeProseDirLinkContext::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrSlUeProseDirLinkContext")
    .SetParent<Object> ()
    .AddConstructor<NrSlUeProseDirLinkContext> ()
  ;
  return tid;
}

TypeId NrSlUeProse::GetTypeId (void)
{
  static TypeId  tid = TypeId ("ns3::NrSlUeProse")
    .SetParent<NrSlUeService> ()
    .AddConstructor<NrSlUeProse> ()
    .AddTraceSource ("PC5SignallingPacketTrace",
                     "Trace fired upon transmission and reception of PC5 Signalling messages",
                     MakeTraceSourceAccessor (&NrSlUeProse::m_pc5SignallingPacketTrace),
                     "ns3::NrSlUeProse::PC5SignallingPacketTracedCallback")
  ;
  return tid;
}

NrSlUeProse::NrSlUeProse ()
{
  NS_LOG_FUNCTION (this);
  m_nrSlUeSvcRrcSapUser = new MemberNrSlUeSvcRrcSapUser<NrSlUeProse> (this);
  m_nrSlUeSvcNasSapUser = new MemberNrSlUeSvcNasSapUser<NrSlUeProse> (this);
  m_nrSlUeProseDirLnkSapUser = new MemberNrSlUeProseDirLnkSapUser<NrSlUeProse> (this);
  m_imsi = 0;
  m_l2Id = 0;
}


NrSlUeProse::~NrSlUeProse (void)
{
  NS_LOG_FUNCTION (this);
}

void
NrSlUeProse::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_nrSlUeSvcRrcSapUser;
  delete m_nrSlUeSvcNasSapUser;
  delete m_nrSlUeProseDirLnkSapUser;
}

NrSlUeSvcRrcSapUser*
NrSlUeProse::GetNrSlUeSvcRrcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeSvcRrcSapUser;
}

void
NrSlUeProse::SetNrSlUeSvcRrcSapProvider (NrSlUeSvcRrcSapProvider* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlUeSvcRrcSapProvider = s;
}


NrSlUeSvcNasSapUser*
NrSlUeProse::GetNrSlUeSvcNasSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeSvcNasSapUser;
}

void
NrSlUeProse::SetNrSlUeSvcNasSapProvider (NrSlUeSvcNasSapProvider* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlUeSvcNasSapProvider = s;
}


void
NrSlUeProse::DoNotifySvcNrSlDataRadioBearerActivated (uint32_t peerL2Id)
{
  NS_LOG_FUNCTION (this << peerL2Id);

  //Get link
  auto it = m_unicastDirectLinks.find (peerL2Id);
  if (it == m_unicastDirectLinks.end ())
    {
      //TODO: Can this happen and should be handled? e.g., Release link before bearer setup completion?
      //      To verify when release procedure is implemented
      NS_FATAL_ERROR ("Could not find the direct link");
    }
  else
    {
      it->second->m_hasPendingSlDrb = false;
      it->second->m_hasActiveSlDrb = true;
    }
}


void NrSlUeProse::ConfigureUnicast ()
{
  NS_LOG_FUNCTION (this);

  //Tell the RRC to inform the MAC to monitor the UE's own L2Id
  m_nrSlUeSvcRrcSapProvider->MonitorSelfL2Id ();

}


void
NrSlUeProse::AddDirectLinkConnection (uint32_t selfL2Id, Ipv4Address selfIp,
                                      uint32_t peerL2Id, bool isInitiating,
                                      bool isRelayConn, uint32_t relayServiceCode)
{
  NS_LOG_FUNCTION (this << selfL2Id << selfIp << peerL2Id << isInitiating << isRelayConn << relayServiceCode);

  bool isIdeal = false;

  NS_ASSERT_MSG (selfL2Id == m_l2Id, "L2Id mismatch.");

  auto it = m_unicastDirectLinks.find (peerL2Id);
  NS_ASSERT_MSG (it == m_unicastDirectLinks.end (), "Direct link " << selfL2Id << "<-->" << peerL2Id << "already exist. "
                 "We currently support only one direct link connection, which should be configured from the scenario once.");

  if (isRelayConn && !isInitiating)  //Relay UE
    {
      auto it = m_l3U2nRelayProvidedSvcs.find (relayServiceCode);
      if (it == m_l3U2nRelayProvidedSvcs.end ())
        {
          NS_FATAL_ERROR ("Could not find the U2N relay service configuration for relay service code: " <<
                          relayServiceCode << ". " <<
                          "Did you configure the service for the relay UEs in the scenario?");
        }
    }

  //Create context to maintain here in the ProSe layer
  Ptr<NrSlUeProseDirLinkContext> context = CreateObject <NrSlUeProseDirLinkContext> ();

  //Create Direct Link instance and set parameters
  Ptr<NrSlUeProseDirectLink> link = CreateObject <NrSlUeProseDirectLink> ();
  link->SetParameters (selfL2Id, peerL2Id, isInitiating, isRelayConn, relayServiceCode, isIdeal, selfIp);

  //Connect SAPs
  link->SetNrSlUeProseDirLnkSapUser (GetNrSlUeProseDirLnkSapUser ());

  context->m_link = link;
  context->m_nrSlUeProseDirLnkSapProvider = link->GetNrSlUeProseDirLnkSapProvider ();
  context->m_ipInfo.selfIpv4Addr = selfIp;

  if (isRelayConn)
    {
      context->m_relayServiceCode = relayServiceCode;
    }

  //Store context
  m_unicastDirectLinks.insert (std::pair <uint32_t, Ptr<NrSlUeProseDirLinkContext> > (peerL2Id, context));

  //Initiate connection establishment procedure if this UE is the initiating UE
  if (isInitiating)
    {
      context->m_link->StartConnectionEstablishment ();
    }
}


NrSlUeProseDirLnkSapUser*
NrSlUeProse::GetNrSlUeProseDirLnkSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeProseDirLnkSapUser;
}


void
NrSlUeProse::DoSendNrSlPc5SMessage (Ptr<Packet> packet, uint32_t dstL2Id,  uint8_t lcId)
{
  NS_LOG_FUNCTION (this);

  //Activate the corresponding SL-SRB for the logical channel, if not active
  auto it = m_activeSlSrbs.find (dstL2Id);
  if (it == m_activeSlSrbs.end ()) //First SL-SRB for this destination
    {
      //Instruct the RRC to activate the SL-SRB
      m_nrSlUeSvcRrcSapProvider->ActivateNrSlSignallingRadioBearer (dstL2Id, lcId);

      //Keep track of it
      std::bitset<4> activeLcs;
      activeLcs[lcId] = true;
      m_activeSlSrbs.insert (std::pair<uint32_t, std::bitset<4> > (dstL2Id, activeLcs));
    }
  else
    {
      if (it->second [lcId] == false) //First SL-SRB for this lcId
        {
          //Instruct the RRC to activate the SL-SRB
          m_nrSlUeSvcRrcSapProvider->ActivateNrSlSignallingRadioBearer (dstL2Id, lcId);
          //Keep track of it
          it->second [lcId] = true;
        }
    }
  m_pc5SignallingPacketTrace (m_l2Id, dstL2Id, true, packet);

  //Pass the message to the RRC
  m_nrSlUeSvcRrcSapProvider->SendNrSlSignalling (packet, dstL2Id, lcId);

}

void
NrSlUeProse::DoReceiveNrSlSignalling (Ptr<Packet> packet, uint32_t srcL2Id)
{
  NS_LOG_FUNCTION (this);

  //This can be PC5-S or a discovery message
  //TODO: How to differentiate them here?

  //If PC5-S for unicast communication:
  auto it = m_unicastDirectLinks.find (srcL2Id);
  if (it == m_unicastDirectLinks.end ())
    {
      NS_LOG_INFO ("Unrecognized peer L2 ID...");
      NS_FATAL_ERROR ("This case is not handled yet. You should configure both UEs on the "
                      "direct link from the scenario.");
      //This will be the case e.g. when the Relay UE receives a request from the Remote UE
      //after the relay discovery procedure.
      //In that case, the Relay UE should:
      //1. Create the corresponding context and direct link instance.
      //2. Pass the packet to the corresponding direct link instance
    }
  else
    {
      NS_LOG_INFO ("Context found!");
      m_pc5SignallingPacketTrace (srcL2Id, m_l2Id, false, packet);

      //Pass the packet to the corresponding direct link instance
      it->second->m_nrSlUeProseDirLnkSapProvider->ReceiveNrSlPc5Message (packet);
    }
}


void
NrSlUeProse::DoNotifyChangeOfDirectLinkState (uint32_t peerL2Id, NrSlUeProseDirLnkSapUser::ChangeOfStateNotification info)
{
  NS_LOG_FUNCTION (this << peerL2Id << info.newStateStr << info.newStateEnum);

  //Get link
  auto it = m_unicastDirectLinks.find (peerL2Id);
  if (it == m_unicastDirectLinks.end ())
    {
      NS_FATAL_ERROR ("Could not find the direct link");
    }

  //Perform action depending on state
  switch (info.newStateEnum)
    {
      case NrSlUeProseDirectLink::INIT:
        NS_LOG_INFO ("INIT");
        break;
      case NrSlUeProseDirectLink::ESTABLISHING:
        NS_LOG_INFO ("ESTABLISHING");
        break;
      case NrSlUeProseDirectLink::ESTABLISHED:
        NS_LOG_INFO ("ESTABLISHED");

        if (!it->second->m_hasActiveSlDrb && !it->second->m_hasPendingSlDrb)
          {

            if (info.relayInfo.isRelayConn)
              {
                NS_LOG_INFO ("info.ipInfo.selfIpv4Addr: " << info.ipInfo.selfIpv4Addr);

                //Depending on the UE Role, we need to tell the NAS to (re)configure the data bearers
                //to have the data packets flowing in the appropriate path
                ConfigureDataRadioBearersForU2nRelay (peerL2Id, info.relayInfo, info.ipInfo);
              }
            else
              {
                NS_LOG_INFO ("Instructing activation of SL-DRB with peer Ipv4 address " << info.ipInfo.peerIpv4Addr);
                ActivateDirectLinkDataRadioBearer (peerL2Id, info.ipInfo);
              }

          }

        break;
      case NrSlUeProseDirectLink::RELEASING:
        NS_LOG_INFO ("RELEASING");
        break;
      case NrSlUeProseDirectLink::RELEASED:
        NS_LOG_INFO ("RELEASED");
        break;
      default:
        NS_FATAL_ERROR ("Invalid state " << info.newStateStr);
    }


}

void
NrSlUeProse::ActivateDirectLinkDataRadioBearer (uint32_t peerL2Id, NrSlUeProseDirLnkSapUser::DirectLinkIpInfo ipInfo)
{
  NS_LOG_FUNCTION (this << peerL2Id << ipInfo.peerIpv4Addr);

  //Get link
  auto it = m_unicastDirectLinks.find (peerL2Id);
  if (it == m_unicastDirectLinks.end ())
    {
      NS_FATAL_ERROR ("Could not find the direct link");
    }
  else
    {
      //Create unicast TFT to be able to transmit to peer UE
      Ptr<LteSlTft> tft;
      tft = Create<LteSlTft> (LteSlTft::Direction::TRANSMIT,
                              LteSlTft::CommType::Unicast,
                              ipInfo.peerIpv4Addr, peerL2Id);
      m_nrSlUeSvcNasSapProvider->ActivateSvcNrSlDataRadioBearer (tft);
      it->second->m_hasPendingSlDrb = true;
    }
}

void
NrSlUeProse::ConfigureDataRadioBearersForU2nRelay (uint32_t peerL2Id,
                                                   NrSlUeProseDirLnkSapUser::DirectLinkRelayInfo relayInfo,
                                                   NrSlUeProseDirLnkSapUser::DirectLinkIpInfo ipInfo)
{
  NS_LOG_FUNCTION (this << peerL2Id << relayInfo.role << ipInfo.peerIpv4Addr);

  uint8_t relayDrbId = 0;
  if (relayInfo.role == NrSlUeProseDirLnkSapUser::RelayUe)
    {
      //Tell the EPC helper to configure the EpcPgwApplication to route the packets
      //directed to the remote UE towards the relay UE
      m_epcHelper->AddRemoteUe (m_imsi, ipInfo.peerIpv4Addr);

      //Find data relay radio bearer id for the service
      auto it = m_l3U2nRelayProvidedSvcs.find (relayInfo.relayServiceCode);
      if (it == m_l3U2nRelayProvidedSvcs.end ())
        {
          NS_FATAL_ERROR ("Could not find the U2N relay service with relay service code :" << relayInfo.relayServiceCode);
        }
      else
        {
          relayDrbId = it->second.relayDrbId;
        }
    }

  //Tell the NAS to (re)configure the UL and SL data bearers to have the data packets
  //flowing in the appropriate path
  m_nrSlUeSvcNasSapProvider->ConfigureNrSlDataRadioBearersForU2nRelay (peerL2Id, relayInfo.role, ipInfo, relayDrbId);

}

void
NrSlUeProse::AddU2nRelayServiceConfiguration (uint32_t relayServiceCode, NrSlL3U2nServiceConfiguration config)
{
  NS_LOG_FUNCTION (this << relayServiceCode << config.relayDrbId);

  auto it = m_l3U2nRelayProvidedSvcs.find (relayServiceCode);
  if (it == m_l3U2nRelayProvidedSvcs.end ())
    {
      //First time
      m_l3U2nRelayProvidedSvcs.insert (std::pair<uint32_t, NrSlL3U2nServiceConfiguration> (relayServiceCode, config));
    }
  else
    {
      //Update
      it->second.relayDrbId = config.relayDrbId;
    }
}


void
NrSlUeProse::SetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

void
NrSlUeProse::SetL2Id (uint32_t l2Id)
{
  NS_LOG_FUNCTION (this << l2Id);
  m_l2Id = l2Id;
}

void
NrSlUeProse::SetEpcHelper (const Ptr<NrPointToPointEpcHelper> &epcHelper)
{
  NS_LOG_FUNCTION (this);
  m_epcHelper = epcHelper;
}

} // namespace ns3
