﻿/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "nr-sl-ue-mac-scheduler-default.h"

#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>
#include "nr-ue-mac.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerDefault");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerDefault);

TypeId
NrSlUeMacSchedulerDefault::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerDefault")
    .SetParent<NrSlUeMacScheduler> ()
    .AddConstructor<NrSlUeMacSchedulerDefault> ()
    .SetGroupName ("nr")
    .AddAttribute ("FixNrSlMcs",
                   "Fix MCS to value set in SetInitialNrSlMcs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrSlUeMacSchedulerDefault::UseFixedNrSlMcs,
                                        &NrSlUeMacSchedulerDefault::IsNrSlMcsFixed),
                   MakeBooleanChecker ())
    .AddAttribute ("InitialNrSlMcs",
                   "The initial value of the MCS used for NR Sidelink",
                   UintegerValue (14),
                   MakeUintegerAccessor (&NrSlUeMacSchedulerDefault::SetInitialNrSlMcs,
                                         &NrSlUeMacSchedulerDefault::GetInitialNrSlMcs),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("NrSlAmc",
                   "The NR SL AMC of this scheduler",
                   PointerValue (),
                   MakePointerAccessor (&NrSlUeMacSchedulerDefault::m_nrSlAmc),
                   MakePointerChecker <NrAmc> ())
    .AddAttribute ("PriorityToSps",
                   "Flag to give scheduling priority to logical channels that are configured with SPS in case of priority tie",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrSlUeMacSchedulerDefault::m_prioToSps),
                   MakeBooleanChecker ())
  ;
  return tid;
}

NrSlUeMacSchedulerDefault::NrSlUeMacSchedulerDefault ()
{
  m_uniformVariable = CreateObject<UniformRandomVariable> ();
  m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable> ();
}

NrSlUeMacSchedulerDefault::~NrSlUeMacSchedulerDefault ()
{
  //just to make sure
  m_dstMap.clear ();
}

void
NrSlUeMacSchedulerDefault::DoCschedUeNrSlLcConfigReq (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id << +params.lcId);

  auto dstInfo = CreateDstInfo (params);
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy
  auto itLcg = lcgMap.find (params.lcGroup);
  auto itLcgEnd = lcgMap.end ();
  if (itLcg == itLcgEnd)
    {
      NS_LOG_DEBUG ("Created new NR SL LCG for destination " << dstInfo->GetDstL2Id () <<
                    " LCG ID =" << static_cast<uint32_t> (params.lcGroup));
      itLcg = dstInfo->Insert (CreateLCG (params.lcGroup));
    }

  itLcg->second->Insert (CreateLC (params));
  NS_LOG_INFO ("Added LC id " << +params.lcId << " in LCG " << +params.lcGroup);
  //send confirmation to UE MAC
  m_nrSlUeMacCschedSapUser->CschedUeNrSlLcConfigCnf (params.lcGroup, params.lcId);
}

std::shared_ptr<NrSlUeMacSchedulerDstInfo>
NrSlUeMacSchedulerDefault::CreateDstInfo (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  std::shared_ptr<NrSlUeMacSchedulerDstInfo> dstInfo = nullptr;
  auto itDst = m_dstMap.find (params.dstL2Id);
  if (itDst == m_dstMap.end ())
    {
      NS_LOG_INFO ("Creating destination info. Destination L2 id " << params.dstL2Id);

      dstInfo = std::make_shared <NrSlUeMacSchedulerDstInfo> (params.dstL2Id);
      dstInfo->SetDstMcs (m_initialNrSlMcs);

      itDst = m_dstMap.insert (std::make_pair (params.dstL2Id, dstInfo)).first;
    }
  else
    {
      NS_LOG_LOGIC ("Doing nothing. You are seeing this because we are adding new LC " << +params.lcId << " for Dst " << params.dstL2Id);
      dstInfo = itDst->second;
    }

  return dstInfo;
}


NrSlLCGPtr
NrSlUeMacSchedulerDefault::CreateLCG (uint8_t lcGroup) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLCG> (new NrSlUeMacSchedulerLCG (lcGroup));
}


NrSlLCPtr
NrSlUeMacSchedulerDefault::CreateLC (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLC> (new NrSlUeMacSchedulerLC (params));
}


void
NrSlUeMacSchedulerDefault::DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id <<
                   static_cast<uint32_t> (params.lcid));

  GetSecond DstInfoOf;
  auto itDst = m_dstMap.find (params.dstL2Id);
  NS_ABORT_MSG_IF (itDst == m_dstMap.end (), "Destination " << params.dstL2Id << " info not found");

  for (const auto &lcg : DstInfoOf (*itDst)->GetNrSlLCG ())
    {
      if (lcg.second->Contains (params.lcid))
        {
          NS_LOG_INFO ("Updating NR SL LC Info: " << params <<
                       " in LCG: " << static_cast<uint32_t> (lcg.first));
          lcg.second->UpdateInfo (params);
          return;
        }
    }
  // Fail miserably because we didn't find any LC
  NS_FATAL_ERROR ("The LC does not exist. Can't update");
}

uint8_t
NrSlUeMacSchedulerDefault::GetRandomReselectionCounter () const
{
  uint8_t min;
  uint8_t max;
  uint16_t periodInt = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ());

  switch(periodInt)
  {
    case 100:
    case 150:
    case 200:
    case 250:
    case 300:
    case 350:
    case 400:
    case 450:
    case 500:
    case 550:
    case 600:
    case 700:
    case 750:
    case 800:
    case 850:
    case 900:
    case 950:
    case 1000:
      min = 5;
      max = 15;
      break;
    default:
      if (periodInt < 100)
        {
          min = GetLowerBoundReselCounter (periodInt);
          max = GetUpperBoundReselCounter (periodInt);
        }
      else
        {
          NS_FATAL_ERROR ("VALUE NOT SUPPORTED!");
        }
      break;
  }

  NS_LOG_DEBUG ("Range to choose random reselection counter. min: " << +min << " max: " << +max);
  return m_ueSelectedUniformVariable->GetInteger (min, max);
}

uint8_t
NrSlUeMacSchedulerDefault::GetLowerBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t lBound = (5 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return lBound;
}

uint8_t
NrSlUeMacSchedulerDefault::GetUpperBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t uBound = (15 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return uBound;
}

void
NrSlUeMacSchedulerDefault::DoSchedUeNrSlTriggerReq (const SfnSf& sfn, const std::deque<uint8_t>& harqIds)
{
  NS_LOG_FUNCTION (this  << harqIds.size ());

  if (harqIds.empty ())
    {
      CheckForGrantsToPublish (sfn);
      return;
    }

  //1. Obtain which destinations and logical channels are in need of scheduling
  std::map<uint32_t, std::vector <uint8_t>> dstsAndLcsToSched;
  GetDstsAndLcsNeedingScheduling (dstsAndLcsToSched);
  if (dstsAndLcsToSched.size () > 0)
    {
      NS_LOG_INFO ("There are " << dstsAndLcsToSched.size () << " destinations needing scheduling");

      //2. Allocate as much of the destinations and logical channels as possible,
      //   following the Logical Channel Prioritization (LCP) procedure
      while (dstsAndLcsToSched.size () > 0)
        {
          AllocationInfo allocationInfo;
          std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> candResources;
          uint32_t dstL2IdtoServe = 0;
          dstL2IdtoServe = LogicalChannelPrioritization (sfn, dstsAndLcsToSched, allocationInfo, candResources);

          NS_LOG_INFO ("Destination L2 Id to allocate: " << dstL2IdtoServe << " Number of LCs: " << allocationInfo.m_allocatedRlcPdus.size ()
                       << " Priority: " << +allocationInfo.m_priority << " Is dynamic: " << allocationInfo.m_isDynamic << " TB size: " << allocationInfo.m_tbSize);
          NS_LOG_INFO ("Resources available (" << candResources.size () << "):");
          for (auto itCandResou : candResources)
            {
              NS_LOG_INFO (itCandResou.sfn << " slSubchannelStart: " << +itCandResou.slSubchannelStart << " slSubchannelSize:" << itCandResou.slSubchannelSize);
            }
          if (dstL2IdtoServe > 0)
            {
              if (candResources.size () > 0 && allocationInfo.m_allocatedRlcPdus.size () > 0)
                {
                  AttemptGrantAllocation (dstL2IdtoServe, candResources, harqIds, allocationInfo);
                  m_reselCounter = 0;
                  m_cResel = 0;

                  //Remove served logical channels from the dstsAndLcsToSched
                  auto itDstsAndLcsToSched = dstsAndLcsToSched.find (dstL2IdtoServe);
                  if (allocationInfo.m_allocatedRlcPdus.size () == itDstsAndLcsToSched->second.size ())
                    {
                      NS_LOG_INFO ("All logical channels of destination " << dstL2IdtoServe << " were allocated");
                      //All LCs where served, remove destination
                      dstsAndLcsToSched.erase(dstL2IdtoServe);

                    }
                  else
                    {
                      NS_LOG_INFO ("Only " << allocationInfo.m_allocatedRlcPdus.size () << "/" << itDstsAndLcsToSched->second.size ()
                                   << " logical channels of destination " << dstL2IdtoServe << " were allocated");
                      //Remove only the LCs that were served
                      for (auto slRlcPduInfo : allocationInfo.m_allocatedRlcPdus)
                        {
                          auto itLcs = itDstsAndLcsToSched->second.begin ();
                          while (itLcs != itDstsAndLcsToSched->second.end ())
                            {
                              if (*itLcs == slRlcPduInfo.lcid )
                                {
                                  NS_LOG_INFO ("- LCID " << slRlcPduInfo.lcid );
                                  itLcs = itDstsAndLcsToSched->second.erase(itLcs);
                                }
                              else
                                {
                                  ++itLcs;
                                }
                            }
                        }
                    }
                }
              else
                {
                  NS_LOG_INFO ("Unable to allocate destination " << dstL2IdtoServe);
                  //It could happen that we are not able to serve this destination
                  //but could serve any of the other destinations needing scheduling.
                  //This case is not currently considered and we stop trying to allocate
                  //destinations at the first one we are not able to serve.
                  break;
                }
            }
          else
            {
              NS_LOG_INFO ("No destination found to serve");
              break;
            }
        }
    }
  else
    {
      NS_LOG_INFO ("No destination needing scheduling");
    }
  CheckForGrantsToPublish (sfn);

}

void
NrSlUeMacSchedulerDefault::DoNotifyNrSlRlcPduDequeue (uint32_t dstL2Id, uint8_t lcId, uint32_t size)
{
  NS_LOG_FUNCTION (this << dstL2Id << +lcId << size);

  const auto itDstInfo = m_dstMap.find (dstL2Id);
  const auto & lcgMap = itDstInfo->second->GetNrSlLCG ();
  lcgMap.begin ()->second->AssignedData (lcId, size);

  return;
}


bool
NrSlUeMacSchedulerDefault::TxResourceReselectionCheck (uint32_t dstL2Id, uint8_t lcId)
{
  NS_LOG_FUNCTION (this << dstL2Id << +lcId);
  const auto itDstInfo = m_dstMap.find (dstL2Id);
  const auto & lcgMap = itDstInfo->second->GetNrSlLCG ();

  bool isLcDynamic = lcgMap.begin ()->second->IsLcDynamic (lcId);
  uint32_t lcBufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (lcId);
  NS_LOG_INFO ("LcId " << +lcId << " buffer size " << lcBufferSize);
  if (lcBufferSize == 0)
    {
      NS_LOG_INFO ("Didn't pass, Empty buffer");
      return false;
    }

  //Check if the LC has a grant
  const auto itGrantInfo = m_grantInfo.find (dstL2Id);
  bool grantFoundForDest = itGrantInfo != m_grantInfo.end () ? true : false;
  bool grantFoundForLc = false;
  std::vector <NrSlUeMacSchedSapUser::NrSlGrantInfo>::iterator itGrantFoundLc;
  if (grantFoundForDest)
    {
      //Look in all the grants of the destination
      for (auto itGrantVector = itGrantInfo->second.begin () ; itGrantVector != itGrantInfo->second.end (); ++itGrantVector)
        {
          if (itGrantVector->slotAllocations.size () == 0)
            {
              continue;
            }
          //Look if any of the RLC PDUs correspond to the LCID
          for (const auto & it : itGrantVector->slotAllocations.begin ()->slRlcPduInfo)
            {
              if (it.lcid == lcId)
                {
                  NS_LOG_DEBUG ("LcId " << +lcId << " already has a grant ");
                  grantFoundForLc = true;
                  break;
                }
            }
          if (grantFoundForLc)
            {
              itGrantFoundLc = itGrantVector;
              break;
            }

        }
    }
  bool pass = false;
  if (isLcDynamic)
    {
      //Currently we do not support grant reevaluation/reselection for dynamic grants.
      //Only the LCs with no grant at the moment and data to transmit will pass the check.
      if (!grantFoundForLc && lcBufferSize > 0)
        {
          NS_LOG_INFO ("Passed, Fresh dynamic grant required");
          pass = true;
        }
    }
  else // SPS
    {
      if (lcBufferSize > 0)
        {
          if (!grantFoundForLc)
            {
              NS_LOG_INFO ("Passed, Fresh SPS grant required");
              pass = true;
            }
          else
            {
              //Currently the only grant reselection that is supported for SPS grants are those governed by
              //the slResoReselCounter, cReselCounter and slProbResourceKeep
              NS_LOG_INFO ("slResoReselCounter " << +itGrantFoundLc->slResoReselCounter <<
                           " cReselCounter " << itGrantFoundLc->cReselCounter);
              if (itGrantFoundLc->slResoReselCounter == 0)
                {
                  if (itGrantFoundLc->cReselCounter > 0)
                    {
                      double randProb = m_ueSelectedUniformVariable->GetValue (0, 1);
                      if (m_slProbResourceKeep > randProb)
                        {
                          NS_LOG_INFO ("m_slProbResourceKeep (" << m_slProbResourceKeep << ") > randProb (" << randProb << ")" <<
                                       ", Keeping the SPS grant, restarting slResoReselCounter");
                          //keeping the resource, reassign the same sidelink resource re-selection
                          //counter we chose while creating the fresh grant
                          itGrantFoundLc->slResoReselCounter = itGrantFoundLc->prevSlResoReselCounter;
                        }
                      else
                        {
                          //Clear the grant.
                          itGrantInfo->second.erase (itGrantFoundLc);
                          NS_LOG_INFO ("Passed, m_slProbResourceKeep (" << m_slProbResourceKeep << ") <= randProb (" << randProb << ")" <<
                                       ", Clearing the SPS grant");
                          pass = true;
                        }
                    }
                  else
                    {
                      //Clear the grant
                      itGrantInfo->second.erase (itGrantFoundLc);
                      NS_LOG_INFO ("Passed, cReselCounter == 0, Clearing the SPS grant");
                      pass = true;
                    }
                }
              else
                {
                  NS_LOG_DEBUG ("slResoReselCounter != 0");
                }
            }
        }
    }
  if (!pass)
    {
      NS_LOG_INFO ("Didn't pass the check");
    }

  return pass;
}

uint32_t
NrSlUeMacSchedulerDefault::LogicalChannelPrioritization (const SfnSf& sfn,
                                                         std::map<uint32_t, std::vector <uint8_t>> dstsAndLcsToSched,
                                                         AllocationInfo &allocationInfo,
                                                         std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> &candResources)

{
  NS_LOG_FUNCTION (this);

  if (dstsAndLcsToSched.size () == 0)
    {
      return 0;
    }
  m_reselCounter = 0;
  m_cResel = 0;

  //At this point all LCs in dstsAndLcsToSched have data to transmit, so we
  //focus on checking the other conditions for the selection and allocation.

  //1. Selection of destination and logical channels to allocate
  //1.1 Select the destination:
  //   - with the LC with the highest priority
  //   - if multiple destination share the same highest priority, select the one with the smallest dstL2Id
  //   Other heuristics that can be developed: closest to PDB, largest queue, longest without allocation, round robin.
  std::map<uint8_t, std::vector <uint32_t>> dstL2IdsbyPrio;
  for (auto & itDst: dstsAndLcsToSched)
    {
      uint8_t lcHighestPrio = 0;
      auto itDstInfo = m_dstMap.find (itDst.first);
      auto & lcgMap = itDstInfo->second->GetNrSlLCG ();
      for (auto & itLc: itDst.second)
        {
          uint8_t lcPriority = lcgMap.begin ()->second->GetLcPriority (itLc);
          NS_LOG_INFO ("Destination L2 ID " << itDst.first << " LCID " << +itLc <<" priority " << +lcPriority <<
                       " buffer size " << lcgMap.begin ()->second->GetTotalSizeOfLC (itLc) <<
                       " dynamic scheduling " << lcgMap.begin ()->second->IsLcDynamic (itLc) <<
                       " RRI " << (lcgMap.begin ()->second->GetLcRri (itLc)).GetMilliSeconds () << " ms");
          if (lcPriority > lcHighestPrio)
            {
              lcHighestPrio = lcPriority;
            }
        }
      auto itDstL2IdsbyPrio = dstL2IdsbyPrio.find (lcHighestPrio);
      if (itDstL2IdsbyPrio == dstL2IdsbyPrio.end ())
        {
          std::vector <uint32_t> dstIds;
          dstIds.emplace_back (itDst.first);
          dstL2IdsbyPrio.emplace (lcHighestPrio, dstIds);
        }
      else
        {
          itDstL2IdsbyPrio->second.emplace_back (itDst.first);
        }
    }
  //The highest priority will be at the rear of the map and the smallest dstL2Id will be at the front of the vector for that priority
  uint8_t dstHighestPrio = dstL2IdsbyPrio.rbegin ()->first;
  uint32_t dstIdWithHighestPrio = dstL2IdsbyPrio.rbegin ()->second.front ();
  NS_LOG_INFO ("Selected destination L2 ID " << dstIdWithHighestPrio
               << " (" << dstL2IdsbyPrio.rbegin ()->second.size () <<"/" <<dstsAndLcsToSched.size ()
               << " destinations shared the highest LC priority value of " << +dstHighestPrio << ")");


  //1.2.Select destination's logical channels that
  // - will have the same grant attributes (scheduling type, scheduling attributes,
  //   and HARQ feedback type) than the LC with highest priority
  // - if multiple LCs with different scheduling type share the same highest priority,
  //   select the one(s) with scheduling type priority indicated by m_prioToSps attribute
  // - if m_prioToSps and multiple LCs with SPS scheduling type and different RRI share the same highest priority,
  //   select the one(s) with RRI equal to the LC with lowest LcId
  // - TODO: how to handle HARQ type in ties
  auto itDstInfo = m_dstMap.find (dstIdWithHighestPrio);
  const auto & lcgMap = itDstInfo->second->GetNrSlLCG ();
  const auto & itDst = dstsAndLcsToSched.find (dstIdWithHighestPrio);
  std::map<uint8_t, std::vector <uint8_t>> lcIdsbyPrio;
  for (auto & itLc: itDst->second)
    {
      uint8_t lcPriority = lcgMap.begin ()->second->GetLcPriority (itLc);
      auto itLcIdsbyPrio = lcIdsbyPrio.find (lcPriority);
      if (itLcIdsbyPrio == lcIdsbyPrio.end ())
        {
          std::vector <uint8_t> lcIds;
          lcIds.emplace_back (itLc);
          lcIdsbyPrio.emplace (lcPriority, lcIds);
        }
      else
        {
          itLcIdsbyPrio->second.emplace_back (itLc);
        }
    }
  //Verify type of scheduling of LCs with highest priority (the one at the rear of the map)
  bool dynamicGrant = true;
  uint16_t nDynLcs = 0;
  uint16_t nSpsLcs = 0;
  if (lcIdsbyPrio.rbegin ()->second.size () > 1)
    {
      for (auto & itLcsHighestPrio: lcIdsbyPrio.rbegin ()->second)
        {
          if (lcgMap.begin ()->second->IsLcDynamic (itLcsHighestPrio))
            {
              nDynLcs++;
            }
          else
            {
              nSpsLcs++;
            }
        }
      if ((m_prioToSps && nSpsLcs > 0) || (!m_prioToSps && nDynLcs == 0 && nSpsLcs > 0))
        {
          dynamicGrant = false;
        }
    }
  else
    {
      dynamicGrant = lcgMap.begin ()->second->IsLcDynamic (lcIdsbyPrio.rbegin ()->second.front ());
    }
  if (dynamicGrant)
    {
      allocationInfo.m_isDynamic = true;
      NS_LOG_INFO ("Selected scheduling type: dynamic grant / per-PDU ");
    }
  else
    {
      allocationInfo.m_isDynamic = false;
      NS_LOG_INFO ("Selected scheduling type: SPS");
    }

  //Remove all LCs that don't have the selected scheduling type
  //Find LcId of reference belonging to the LC with selected scheduling type, highest priority and smallest LcId
  uint16_t nLcs = 0;
  uint16_t nRemainingLcs = 0;
  uint8_t lcIdOfRef = 0;
  for (auto itlcIdsbyPrio = lcIdsbyPrio.rbegin(); itlcIdsbyPrio != lcIdsbyPrio.rend (); ++itlcIdsbyPrio)
    {
      uint8_t lowestLcId = std::numeric_limits <uint8_t>::max ();
      for (auto itLcs = itlcIdsbyPrio->second.begin () ; itLcs != itlcIdsbyPrio->second.end ();)
        {
          nLcs++;
          if (lcgMap.begin ()->second->IsLcDynamic (*itLcs) != dynamicGrant)
            {
              itLcs = itlcIdsbyPrio->second.erase (itLcs);
            }
          else
            {
              if (*itLcs < lowestLcId)
                {
                  lowestLcId = *itLcs;
                }
              ++itLcs;
              nRemainingLcs++;
            }
        }
      if (itlcIdsbyPrio->second.size () == 0)
        {
          itlcIdsbyPrio = std::reverse_iterator (lcIdsbyPrio.erase (--itlcIdsbyPrio.base()));
        }

      if (lowestLcId != std::numeric_limits <uint8_t>::max () && lcIdOfRef == 0)
        {
          lcIdOfRef = lowestLcId;
        }
    }
  //If SPS, remove all LCs with RRI different than the lcIdOfRef, and assign re-selection counters
  if (!dynamicGrant)
    {
      for (auto itlcIdsbyPrio = lcIdsbyPrio.begin(); itlcIdsbyPrio != lcIdsbyPrio.end (); ++itlcIdsbyPrio)
        {
          for (auto itLcs = itlcIdsbyPrio->second.begin () ; itLcs != itlcIdsbyPrio->second.end ();)
            {
              if (lcgMap.begin ()->second->GetLcRri (*itLcs) != lcgMap.begin ()->second->GetLcRri (lcIdOfRef))
                {
                  itLcs = itlcIdsbyPrio->second.erase (itLcs);
                  nRemainingLcs --;
                }
              else
                {
                  ++itLcs;
                }
            }
          if (itlcIdsbyPrio->second.size () == 0)
            {
              itlcIdsbyPrio = lcIdsbyPrio.erase (itlcIdsbyPrio);

            }
        }

      allocationInfo.m_rri = lcgMap.begin ()->second->GetLcRri (lcIdOfRef);
      //Do it here because we need m_cResel for getting the candidate resources from the MAC
      m_reselCounter = GetRandomReselectionCounter ();
      m_cResel = m_reselCounter * 10;
      NS_LOG_INFO ("SPS Reselection counters: m_reselCounter " << +m_reselCounter << " m_cResel " << m_cResel);
    }
  allocationInfo.m_priority = lcgMap.begin ()->second->GetLcPriority (lcIdOfRef);
  NS_LOG_INFO ("Number of LCs to attempt allocation for the selected destination: "<< nRemainingLcs << "/" << nLcs <<
               ". LcId of reference " << +lcIdOfRef);

  //2. Allocation of sidelink resources
  NS_LOG_INFO ("Getting resources" );
  //2.1 Select which logical channels can be allocated
  std::map<uint8_t, std::vector <uint8_t>> selectedLcs = lcIdsbyPrio;
  std::queue<std::vector <uint8_t>> allocQueue;
  uint32_t bufferSize = 0;
  uint32_t nLcsInQueue = 0;
  uint32_t candResoTbSize = 0;
  uint8_t dstMcs = itDstInfo->second->GetDstMcs ();
  // For further study:  The number of symbols per slot will be variable
  // depending  on whether PSFCH is present, but for the moment, PSFCH is
  // not modeled.
  uint16_t symbolsPerSlot = m_nrUeMac->GetNrSlPsschSymbolsPerSlot ();
  uint16_t subChannelSize = m_nrUeMac->GetNrSlSubChSize ();
  auto rItSelectedLcs = selectedLcs.rbegin (); //reverse iterator
  while (selectedLcs.size () > 0)
    {
      allocQueue.emplace (rItSelectedLcs->second);
      //Calculate buffer size of LCs just pushed in the queue
      uint32_t currBufferSize = 0;
      for (auto & itLc : rItSelectedLcs->second)
        {
          currBufferSize = currBufferSize + lcgMap.begin ()->second->GetTotalSizeOfLC (itLc);
        }
      nLcsInQueue = nLcsInQueue + rItSelectedLcs->second.size ();
      //Calculate buffer size of all LCs currently in the queue
      bufferSize = bufferSize + currBufferSize;

      //Calculate number of needed subchannels
      // The following do/while loop iterates until providing a transport
      // block size large enough to cover the buffer size plus 5 bytes for
      // SCI-2A information.
      uint16_t lSubch = 0;
      uint32_t tbSize = 0;
      do
        {
          lSubch++;
          tbSize = CalculateTbSize (GetNrSlAmc (), dstMcs, symbolsPerSlot, lSubch, subChannelSize);
        }
      while (tbSize < bufferSize + 5  && lSubch < GetTotalSubCh ());

      NS_LOG_DEBUG ("Trying " << nLcsInQueue << " LCs with total buffer size of " << bufferSize <<
                    " bytes in " << lSubch << " subchannels for a TB size of " << tbSize << " bytes");

      //All LCs in the set should have the same attributes than the lcIdOfRef
      NrSlTransmissionParams params {lcgMap.begin ()->second->GetLcPriority (lcIdOfRef),
        lcgMap.begin ()->second->GetLcPdb (lcIdOfRef), lSubch,
        lcgMap.begin ()->second->GetLcRri (lcIdOfRef), m_cResel};
      std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> filteredReso;
      filteredReso = FilterTxOpportunities (m_nrUeMac->GetNrSlAvailableResources (sfn, params));
      if (filteredReso.size () == 0)
        {
          NS_LOG_DEBUG ("Resources not found");
          break;
        }
      else
        {
          NS_LOG_DEBUG ("Resources found");
          candResoTbSize = tbSize;
          candResources = filteredReso;
        }
      rItSelectedLcs = std::reverse_iterator (selectedLcs.erase (--rItSelectedLcs.base ()));
    }
  if (candResources.size () == 0)
    {
      NS_LOG_INFO ("Unable to find resources");
      return 0;
    }
  allocationInfo.m_tbSize = candResoTbSize;
  NS_LOG_INFO ("Destination L2 ID " << dstIdWithHighestPrio << " got "  << candResources.size () <<
               " resources (of TB size " << candResoTbSize << ")" <<
               " available to allocate " << nLcsInQueue << " LCs with total buffer size of " << bufferSize << " bytes");

  //2.2 Allocate the resources to logical channels
  uint32_t allocatedSize = 0;
  while (allocQueue.size () > 0)
    {
      //All LCs of the same priority are served equally
      //Find how much to allocate to each
      uint32_t minBufferSize =  std::numeric_limits <uint32_t>::max ();
      uint32_t toServeBufferSize = 0;
      for (auto itLc : allocQueue.front ())
        {
          if ( lcgMap.begin ()->second->GetTotalSizeOfLC (itLc) < minBufferSize)
            {
              minBufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (itLc);
            }
        }
      toServeBufferSize = minBufferSize;
      if (allocQueue.front ().size () * toServeBufferSize > candResoTbSize -  allocatedSize - 5) // 5 bytes of SCI-2A
        {
          toServeBufferSize = std::floor ((candResoTbSize -  allocatedSize - 5)/allocQueue.front ().size ());
        }
      if (toServeBufferSize > 0)
        {
          //Allocate
          for (auto itLc : allocQueue.front ())
            {
              SlRlcPduInfo slRlcPduInfo (itLc, toServeBufferSize);
              allocationInfo.m_allocatedRlcPdus.push_back (slRlcPduInfo);
              NS_LOG_INFO ("LC ID " << +itLc << " will be allocated " << toServeBufferSize << " bytes");
              allocatedSize = allocatedSize + toServeBufferSize;
            }
        }
      else
        {
          break;
        }

      allocQueue.pop ();
    }



  return dstIdWithHighestPrio;
}

void
NrSlUeMacSchedulerDefault::GetDstsAndLcsNeedingScheduling (std::map<uint32_t, std::vector <uint8_t>> &dstsAndLcsToSched)
{
  NS_LOG_FUNCTION (this);
  for (auto & itDstInfo : m_dstMap)
    {
      const auto & lcgMap = itDstInfo.second->GetNrSlLCG (); //Map of unique_ptr should not copy
      std::vector<uint8_t> lcVector = lcgMap.begin ()->second->GetLCId ();
      std::vector<uint8_t> passedLcsVector;
      for (auto & itLcId : lcVector)
        {
          if (TxResourceReselectionCheck (itDstInfo.first, itLcId))
            {
              passedLcsVector.emplace_back (itLcId);
            }
        }
      if (passedLcsVector.size () > 0)
        {
          dstsAndLcsToSched.emplace (itDstInfo.first, passedLcsVector);
        }
      NS_LOG_INFO ("Destination L2 ID " << itDstInfo.first << " has " << passedLcsVector.size () <<" LCs needing scheduling");

    }
}

uint8_t
NrSlUeMacSchedulerDefault::GetUnusedHarqId (const std::deque<uint8_t>& ids)
{
  NS_LOG_FUNCTION (this << ids.size ());
  uint8_t id = std::numeric_limits <uint8_t>::max ();
  std::deque<uint8_t> idsInUse;

  if (ids.size () > 0)
    {
      for (auto & itGrantInfo : m_grantInfo)
        {
          for (auto & itGrantVector : itGrantInfo.second)
            {
              idsInUse.push_back (itGrantVector.nrSlHarqId);
            }
        }
      if (idsInUse.size () > 0)
        {
          for (auto & itId : ids)
            {
              bool found = false;
              for (auto & itIdsInUse : idsInUse)
                {
                  if (itId == itIdsInUse)
                    {
                      found = true;
                      break;
                    }
                }
              if (!found)
                {
                  id = itId;
                  break;
                }
            }
        }
      else
        {
          id = ids.front ();
        }
    }

  return id;
}

void
NrSlUeMacSchedulerDefault::AttemptGrantAllocation (uint32_t dstL2Id,
                                                    const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& candResources,
                                                    const std::deque<uint8_t>& harqIds,
                                                    AllocationInfo allocationInfo)
{
  NS_LOG_FUNCTION (this << dstL2Id);

  std::set<NrSlSlotAlloc> allocList;

  const auto itDstInfo = m_dstMap.find (dstL2Id);
  bool allocated = DoNrSlAllocation (candResources, itDstInfo->second, allocList, allocationInfo);

  if (!allocated)
    {
      return;
    }

  if (allocationInfo.m_isDynamic)
    {
      CreateSinglePduGrant (allocList, harqIds);
    }
  else
    {
      CreateSpsGrant (allocList, harqIds, allocationInfo.m_rri);
    }
}

void
NrSlUeMacSchedulerDefault::CreateSpsGrant (const std::set<NrSlSlotAlloc>& slotAllocList, const std::deque<uint8_t>& ids, Time rri)
{
  NS_LOG_FUNCTION (this << ids.size ());
  auto itGrantInfo = m_grantInfo.find (slotAllocList.begin ()->dstL2Id);

  if (itGrantInfo == m_grantInfo.end ())
    {
      NS_LOG_DEBUG ("New destination " << slotAllocList.begin ()->dstL2Id);
      uint8_t harqId = GetUnusedHarqId (ids) ; // Assign new HARQ process ID
      if (harqId == std::numeric_limits <uint8_t>::max ())
        {
          NS_LOG_DEBUG ("Unable to create grant, HARQ Id not available");
          return;
        }
      NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSpsGrantInfo (slotAllocList, rri);
      grant.nrSlHarqId = harqId;
      std::vector <NrSlUeMacSchedSapUser::NrSlGrantInfo> grantVector;
      grantVector.push_back (grant);
      itGrantInfo = m_grantInfo.emplace (std::make_pair (slotAllocList.begin ()->dstL2Id, grantVector)).first;
      NS_LOG_DEBUG ("New grant created. HARQ ID " << +grant.nrSlHarqId);
    }
  else
    {
      NS_LOG_DEBUG ("Destination " << slotAllocList.begin ()->dstL2Id << " found");
      //Destination exists
      bool grantFound = false;
      auto itGrantVector =  itGrantInfo->second.begin ();
      for (; itGrantVector != itGrantInfo->second.end (); ++itGrantVector)
        {
          if (itGrantVector->rri == rri &&
              itGrantVector->slotAllocations.begin ()->slRlcPduInfo.size () == slotAllocList.begin ()->slRlcPduInfo.size ())
            {
              uint16_t foundLcs = 0;
              for (auto itGrantRlcPdu : itGrantVector->slotAllocations.begin ()->slRlcPduInfo)
                {
                  for (auto itNewRlcPdu : slotAllocList.begin ()->slRlcPduInfo)
                    {
                      if (itGrantRlcPdu.lcid == itNewRlcPdu.lcid)
                        {
                          foundLcs ++;
                          break;
                        }
                    }
                }
              if(foundLcs == itGrantVector->slotAllocations.begin ()->slRlcPduInfo.size ())
                {
                  grantFound = true;
                  break;
                  // itGrantVector normally points to the found grant at this point
                }
            }
        }
      if (grantFound)
        {
          //Update
          NS_ASSERT_MSG (itGrantVector->slResoReselCounter == 0, "Sidelink resource counter must be zero before assigning new grant for dst " << slotAllocList.begin ()->dstL2Id);
          uint8_t prevHarqId = itGrantVector->nrSlHarqId;
          NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSpsGrantInfo (slotAllocList, rri);
          *itGrantVector = grant;
          itGrantVector->nrSlHarqId = prevHarqId; // Preserve previous ID
          NS_LOG_DEBUG ("Updated grant. HARQ ID " << itGrantVector->nrSlHarqId);

        }
      else
        {
          //Insert
          uint8_t harqId = GetUnusedHarqId (ids) ; // Assign new HARQ process ID
          if (harqId == std::numeric_limits <uint8_t>::max ())
            {
              NS_LOG_DEBUG ("Unable to create grant, HARQ Id not available");
              return;
            }
          NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSpsGrantInfo (slotAllocList, rri);
          grant.nrSlHarqId = harqId;
          itGrantInfo->second.push_back (grant);
          NS_LOG_DEBUG ("New grant created. HARQ ID " << +grant.nrSlHarqId);
        }
    }
}

void
NrSlUeMacSchedulerDefault::CreateSinglePduGrant (const std::set<NrSlSlotAlloc>& slotAllocList, const std::deque<uint8_t>& ids)
{
  NS_LOG_FUNCTION (this << ids.size ());
  auto itGrantInfo = m_grantInfo.find (slotAllocList.begin ()->dstL2Id);

  if (itGrantInfo == m_grantInfo.end ())
    {
      //New destination
      NS_LOG_DEBUG ("New destination " << slotAllocList.begin ()->dstL2Id);
      uint8_t harqId = GetUnusedHarqId (ids) ; // Assign new HARQ process ID
      if (harqId == std::numeric_limits <uint8_t>::max ())
        {
          NS_LOG_DEBUG ("Unable to create grant, HARQ Id not available");
          return;
        }
      NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSinglePduGrantInfo (slotAllocList);
      grant.nrSlHarqId = harqId;
      std::vector <NrSlUeMacSchedSapUser::NrSlGrantInfo> grantVector;
      grantVector.push_back (grant);
      itGrantInfo = m_grantInfo.emplace (std::make_pair (slotAllocList.begin ()->dstL2Id, grantVector)).first;
      NS_LOG_DEBUG ("New grant. HARQ ID " << +grant.nrSlHarqId);

    }
  else
    {
      //Destination exists
      NS_LOG_DEBUG ("Destination " << slotAllocList.begin ()->dstL2Id << " found");
      bool grantFound = false;
      auto itGrantVector =  itGrantInfo->second.begin ();
      for (; itGrantVector != itGrantInfo->second.end (); ++itGrantVector)
        {
          if (itGrantVector->slotAllocations.begin ()->slRlcPduInfo.size () == slotAllocList.begin ()->slRlcPduInfo.size ())
            {
              uint16_t foundLcs = 0;
              for (auto itGrantRlcPdu : itGrantVector->slotAllocations.begin ()->slRlcPduInfo)
                {
                  for (auto itNewRlcPdu : slotAllocList.begin ()->slRlcPduInfo)
                    {
                      if (itGrantRlcPdu.lcid == itNewRlcPdu.lcid)
                        {
                          foundLcs ++;
                          break;
                        }
                    }
                }
              if(foundLcs == itGrantVector->slotAllocations.begin ()->slRlcPduInfo.size ())
                {
                  grantFound = true;
                  break;
                  // itGrantVector normally points to the found grant at this point
                }
            }
        }
      if (grantFound)
        {
          //Update
          uint8_t prevHarqId = itGrantVector->nrSlHarqId;
          NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSinglePduGrantInfo (slotAllocList);
          *itGrantVector = grant;
          itGrantVector->nrSlHarqId = prevHarqId; // Preserve previous ID
          NS_LOG_DEBUG ("Updating grant. HARQ ID " << itGrantVector->nrSlHarqId);
        }
      else
        {
          //Insert
          uint8_t harqId = GetUnusedHarqId (ids) ; // Assign new HARQ process ID
          if (harqId == std::numeric_limits <uint8_t>::max ())
            {
              NS_LOG_DEBUG ("Unable to create grant, HARQ Id not available");
              return;
            }
          NrSlUeMacSchedSapUser::NrSlGrantInfo grant = CreateSinglePduGrantInfo (slotAllocList);
          grant.nrSlHarqId = harqId;
          itGrantInfo->second.push_back (grant);
          NS_LOG_DEBUG ("New grant. HARQ ID " << +grant.nrSlHarqId);

        }
    }
}

NrSlUeMacSchedSapUser::NrSlGrantInfo
NrSlUeMacSchedulerDefault::CreateSpsGrantInfo (const std::set<NrSlSlotAlloc>& slotAllocList, Time rri)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG ((m_reselCounter != 0), "Can not create grants with 0 Resource selection counter");
  NS_ASSERT_MSG ((m_cResel != 0), "Can not create grants with 0 cResel counter");

  NS_LOG_DEBUG ("Creating SPS grants for dstL2Id " << slotAllocList.begin ()->dstL2Id);
  NS_LOG_DEBUG ("Resource reservation interval " << rri.GetMilliSeconds () << " ms");
  NS_LOG_DEBUG ("Resel Counter " << +m_reselCounter << " and cResel " << m_cResel);

  uint16_t resPeriodSlots = m_nrUeMac->GetResvPeriodInSlots (rri);
  NrSlUeMacSchedSapUser::NrSlGrantInfo grant;

  grant.cReselCounter = m_cResel;
  //save reselCounter to be used if probability of keeping the resource would
  //be higher than the configured one
  grant.prevSlResoReselCounter = m_reselCounter;
  grant.slResoReselCounter = m_reselCounter;

  // if further IDs are needed and the std::deque needs to be popped from
  // front, need to copy the std::deque to remove its constness
  grant.nSelected = static_cast<uint8_t>(slotAllocList.size ());
  grant.rri = rri;
  NS_LOG_DEBUG ("nSelected = " << +grant.nSelected);

  for (uint16_t i = 0; i < m_cResel; i++)
    {
      for (const auto &it : slotAllocList)
        {
          auto slAlloc = it;
          slAlloc.sfn.Add (i * resPeriodSlots);

          if (slAlloc.ndi == 1)
            {
              NS_LOG_DEBUG ("First tx at : Frame = " << slAlloc.sfn.GetFrame ()
                            << " SF = " << +slAlloc.sfn.GetSubframe ()
                            << " slot = " << +slAlloc.sfn.GetSlot ());
            }
          else
            {
              NS_LOG_DEBUG ("Rtx at : Frame = " << slAlloc.sfn.GetFrame ()
                            << " SF = " << +slAlloc.sfn.GetSubframe ()
                            << " slot = " << +slAlloc.sfn.GetSlot ());
            }
          bool insertStatus = grant.slotAllocations.emplace (slAlloc).second;
          NS_ASSERT_MSG (insertStatus, "slot allocation already exist");
        }
    }

  return grant;
}

NrSlUeMacSchedSapUser::NrSlGrantInfo
NrSlUeMacSchedulerDefault::CreateSinglePduGrantInfo (const std::set<NrSlSlotAlloc>& slotAllocList)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Creating single-PDU grant for dstL2Id " << slotAllocList.begin ()->dstL2Id);

  NrSlUeMacSchedSapUser::NrSlGrantInfo grant;
  grant.nSelected = static_cast<uint8_t>(slotAllocList.size ());
  grant.isDynamic = true;

  NS_LOG_DEBUG ("nSelected = " << +grant.nSelected);

  for (const auto &it : slotAllocList)
    {
      auto slAlloc = it;
      if (slAlloc.ndi == 1)
        {
          NS_LOG_DEBUG ("First tx at : Frame = " << slAlloc.sfn.GetFrame ()
                        << " SF = " << +slAlloc.sfn.GetSubframe ()
                        << " slot = " << +slAlloc.sfn.GetSlot ());
        }
      else
        {
          NS_LOG_DEBUG ("Rtx at : Frame = " << slAlloc.sfn.GetFrame ()
                        << " SF = " << +slAlloc.sfn.GetSubframe ()
                        << " slot = " << +slAlloc.sfn.GetSlot ());
        }
      bool insertStatus = grant.slotAllocations.emplace (slAlloc).second;
      NS_ASSERT_MSG (insertStatus, "slot allocation already exist");
    }


  return grant;
}

void
NrSlUeMacSchedulerDefault::CheckForGrantsToPublish (const SfnSf& sfn)
{
  NS_LOG_FUNCTION (this << sfn.Normalize ());
  for (auto & itGrantInfo : m_grantInfo)
    {
      for (auto itGrantVector = itGrantInfo.second.begin () ; itGrantVector != itGrantInfo.second.end ();)
        {
          if (!itGrantVector->isDynamic && itGrantVector->slResoReselCounter == 0)
            {
              ++itGrantVector;
              continue;
            }

          if (itGrantVector->slotAllocations.begin ()->sfn.Normalize () > sfn.Normalize () + m_t1)
            {
              ++itGrantVector;
              continue;
            }
          // The next set of slots (NDI + any retransmissions) should be added
          // to a grant, and deleted from m_grantInfo
          auto slotIt = itGrantVector->slotAllocations.begin ();
          NS_ASSERT_MSG (slotIt->ndi == 1, "New data indication not found");
          NS_ASSERT_MSG (slotIt->sfn.Normalize () >= sfn.Normalize (), "Stale slot in m_grantInfo");
          NrSlSlotAlloc currentSlot = *slotIt;
          NS_LOG_INFO ("Slot at : Frame = " << currentSlot.sfn.GetFrame ()
                       << " SF = " << +currentSlot.sfn.GetSubframe ()
                       << " slot = " << currentSlot.sfn.GetSlot ());
          uint32_t tbSize = 0;
          //sum all the assigned bytes to each LC of this destination
          for (const auto & it : currentSlot.slRlcPduInfo)
            {
              NS_LOG_DEBUG ("LC " << static_cast <uint16_t> (it.lcid) << " was assigned " << it.size << " bytes");
              tbSize += it.size;
            }
          itGrantVector->tbTxCounter = 1;
          NrSlUeMacSchedSapUser::NrSlGrant grant;
          grant.nrSlHarqId = itGrantVector->nrSlHarqId;
          grant.nSelected = itGrantVector->nSelected;
          grant.tbTxCounter = itGrantVector->tbTxCounter;
          grant.tbSize = tbSize;
          grant.rri = itGrantVector->rri;
          // Add the NDI slot and retransmissions to the set of slot allocations
          grant.slotAllocations.emplace (currentSlot);
          itGrantVector->slotAllocations.erase (slotIt);
          // Add any retransmission slots and erase them
          slotIt = itGrantVector->slotAllocations.begin ();
          while (slotIt != itGrantVector->slotAllocations.end () && slotIt->ndi == 0)
            {
              NrSlSlotAlloc nextSlot = *slotIt;
              grant.slotAllocations.emplace (nextSlot);
              itGrantVector->slotAllocations.erase (slotIt);
              slotIt = itGrantVector->slotAllocations.begin ();
            }
          m_nrSlUeMacSchedSapUser->SchedUeNrSlConfigInd (currentSlot.dstL2Id, grant);

          if (itGrantVector->isDynamic)
            {
              itGrantVector = itGrantInfo.second.erase (itGrantVector);
            }
          else
            {
              // Decrement counters for reselection
              --itGrantVector->slResoReselCounter;
              --itGrantVector->cReselCounter;
              ++itGrantVector;
            }
        }
    }
}

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrSlUeMacSchedulerDefault::FilterTxOpportunities (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOppr)
{
  NS_LOG_FUNCTION (this);

  if (txOppr.empty ())
    { 
      return txOppr;
    }

  NrSlSlotAlloc dummyAlloc;
  for (const auto & itDst : m_grantInfo)
    {
      for (auto itGrantVector = itDst.second.begin () ; itGrantVector != itDst.second.end (); ++itGrantVector)
        {
          for (auto itGrantAlloc = itGrantVector->slotAllocations.begin (); itGrantAlloc != itGrantVector->slotAllocations.end (); itGrantAlloc ++)
            {
              auto itTxOppr = txOppr.begin ();
              while (itTxOppr != txOppr.end ())
                {
                  //Currently the PHY doesn't handle multiple PSSCH transmissions in the same slot
                  //Thus, we need to remove all candidate resources belonging to the same slot than a granted resource
                  if (itGrantAlloc->sfn == itTxOppr->sfn)
                    {
                      itTxOppr = txOppr.erase (itTxOppr);
                    }
                  else
                    {
                      ++itTxOppr;
                    }
                }
            }
        }
    }
  
  return txOppr;
}

// XXX the below is a candidate for removal
void
NrSlUeMacSchedulerDefault::DoSlotIndication (SfnSf sfn, bool isSidelinkSlot)
{
  NS_LOG_FUNCTION (this << sfn.Normalize () << isSidelinkSlot);
  if (!isSidelinkSlot)
    {
      return;
    }
}

uint8_t
NrSlUeMacSchedulerDefault::GetTotalSubCh () const
{
  return m_nrSlUeMacSchedSapUser->GetTotalSubCh ();
}

uint8_t
NrSlUeMacSchedulerDefault::GetSlMaxTxTransNumPssch () const
{
  return m_nrSlUeMacSchedSapUser->GetSlMaxTxTransNumPssch ();
}


void
NrSlUeMacSchedulerDefault::InstallNrSlAmc (const Ptr<NrAmc> &nrSlAmc)
{
  NS_LOG_FUNCTION (this);
  m_nrSlAmc = nrSlAmc;
  //In NR it does not have any impact
  m_nrSlAmc->SetUlMode ();
}

Ptr<const NrAmc>
NrSlUeMacSchedulerDefault::GetNrSlAmc () const
{
  NS_LOG_FUNCTION (this);
  return m_nrSlAmc;
}

void
NrSlUeMacSchedulerDefault::UseFixedNrSlMcs (bool fixMcs)
{
  NS_LOG_FUNCTION (this);
  m_fixedNrSlMcs = fixMcs;
}

bool
NrSlUeMacSchedulerDefault::IsNrSlMcsFixed () const
{
  NS_LOG_FUNCTION (this);
  return m_fixedNrSlMcs;
}

void
NrSlUeMacSchedulerDefault::SetInitialNrSlMcs (uint8_t mcs)
{
  NS_LOG_FUNCTION (this);
  m_initialNrSlMcs = mcs;
}

uint8_t
NrSlUeMacSchedulerDefault::GetInitialNrSlMcs () const
{
  NS_LOG_FUNCTION (this);
  return m_initialNrSlMcs;
}



uint8_t
NrSlUeMacSchedulerDefault::GetRv (uint8_t txNumTb) const
{
  NS_LOG_FUNCTION (this << +txNumTb);
  uint8_t modulo  = txNumTb % 4;
  //we assume rvid = 0, so RV would take 0, 2, 3, 1
  //see TS 38.21 table 6.1.2.1-2
  uint8_t rv = 0;
  switch (modulo)
  {
    case 0:
      rv = 0;
      break;
    case 1:
      rv = 2;
      break;
    case 2:
      rv = 3;
      break;
    case 3:
      rv = 1;
      break;
    default:
      NS_ABORT_MSG ("Wrong modulo result to deduce RV");
  }

  return rv;
}

void
NrSlUeMacSchedulerDefault::SetNrUeMac (Ptr<NrUeMac> nrUeMac)
{
  NS_LOG_FUNCTION (this << nrUeMac);
  m_nrUeMac = nrUeMac;
}

int64_t
NrSlUeMacSchedulerDefault::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformVariable->SetStream (stream);
  m_ueSelectedUniformVariable->SetStream (stream + 1);
  return 2;
}

void
NrSlUeMacSchedulerDefault::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_nrUeMac = nullptr;
}

uint32_t
NrSlUeMacSchedulerDefault::CalculateTbSize (Ptr<const NrAmc> nrAmc, uint8_t dstMcs, uint16_t symbolsPerSlot, uint16_t availableSubChannels, uint16_t subChannelSize) const
{
  NS_LOG_FUNCTION (this << nrAmc << dstMcs << symbolsPerSlot << availableSubChannels << subChannelSize);
  NS_ASSERT_MSG (availableSubChannels > 0, "Must have at least one available subchannel");
  NS_ASSERT_MSG (subChannelSize > 0, "Must have non-zero subChannelSize");
  NS_ASSERT_MSG (symbolsPerSlot <= 14, "Invalid number of symbols per slot");
  return nrAmc->CalculateTbSize (dstMcs, subChannelSize * availableSubChannels * symbolsPerSlot);
}


bool
NrSlUeMacSchedulerDefault::DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& candResources,
                                            const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                            std::set<NrSlSlotAlloc> &slotAllocList,
                                            AllocationInfo allocationInfo)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;
  NS_ASSERT_MSG (candResources.size () > 0, "Scheduler received an empty resource list from UE MAC");
  NS_ASSERT_MSG (IsNrSlMcsFixed (), "Attribute FixNrSlMcs must be true for NrSlUeMacSchedulerDefault scheduler");


  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> selectedTxOpps = RandomlySelectResources (candResources);
  NS_ASSERT_MSG (selectedTxOpps.size () > 0, "Scheduler should select at least 1 slot from txOpps");
  allocated = true;
  auto itTxOpps = selectedTxOpps.cbegin ();
  for (; itTxOpps != selectedTxOpps.cend (); ++itTxOpps)
    {

      NrSlSlotAlloc slotAlloc;
      slotAlloc.sfn = itTxOpps->sfn;
      slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
      slotAlloc.priority = allocationInfo.m_priority;
      slotAlloc.slRlcPduInfo = allocationInfo.m_allocatedRlcPdus;
      slotAlloc.mcs = dstInfo->GetDstMcs ();
      //PSCCH
      slotAlloc.numSlPscchRbs = itTxOpps->numSlPscchRbs;
      slotAlloc.slPscchSymStart = itTxOpps->slPscchSymStart;
      slotAlloc.slPscchSymLength = itTxOpps->slPscchSymLength;
      //PSSCH
      slotAlloc.slPsschSymStart = itTxOpps->slPsschSymStart;
      slotAlloc.slPsschSymLength = itTxOpps->slPsschSymLength;
      slotAlloc.slPsschSubChStart = itTxOpps->slSubchannelStart;
      slotAlloc.slPsschSubChLength = itTxOpps->slSubchannelLength;
      slotAlloc.maxNumPerReserve = itTxOpps->slMaxNumPerReserve;
      slotAlloc.ndi = slotAllocList.empty () == true ? 1 : 0;
      slotAlloc.rv = GetRv (static_cast<uint8_t>(slotAllocList.size ()));
      if (static_cast<uint16_t>(slotAllocList.size ()) % itTxOpps->slMaxNumPerReserve == 0)
        {
          slotAlloc.txSci1A = true;
          if (slotAllocList.size () + itTxOpps->slMaxNumPerReserve <= selectedTxOpps.size ())
            {
              slotAlloc.slotNumInd = itTxOpps->slMaxNumPerReserve;
            }
          else
            {
              slotAlloc.slotNumInd = selectedTxOpps.size () - slotAllocList.size ();
            }
        }
      else
        {
          slotAlloc.txSci1A = false;
          //Slot, which does not carry SCI 1-A can not indicate future TXs
          slotAlloc.slotNumInd = 0;
        }

      slotAllocList.emplace (slotAlloc);
    }

  return allocated;
}


bool
NrSlUeMacSchedulerDefault::OverlappedSlots (const std::list<NrSlUeMacSchedSapProvider::NrSlSlotInfo>& resources,
  const NrSlUeMacSchedSapProvider::NrSlSlotInfo& candidate) const
{
  for (const auto & it : resources)
    {
      if (it.sfn == candidate.sfn)
        {
          return true;
        }
    }
  return false;
}

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrSlUeMacSchedulerDefault::RandomlySelectResources (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this << txOpps.size ());

  uint8_t totalTx = GetSlMaxTxTransNumPssch ();
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> newTxOpps;

  if (txOpps.size () > totalTx)
    {
      while (newTxOpps.size () != totalTx)
        {
          auto txOppsIt = txOpps.begin ();
          // Walk through list until the random element is reached
          std::advance (txOppsIt, m_uniformVariable->GetInteger (0, txOpps.size () - 1));
          if (!OverlappedSlots (newTxOpps, *txOppsIt))
            {
              //copy the randomly selected slot info into the new list
              newTxOpps.emplace_back (*txOppsIt);
            }
          //erase the selected one from the list
          txOppsIt = txOpps.erase (txOppsIt);
          if (txOppsIt == txOpps.end ())
            {
              break;
            }
        }
    }
  else
    {
      //We need to remove overlapped slots here too
      auto txOppsIt = txOpps.begin ();
      while (txOppsIt != txOpps.end())
        {
          if (!OverlappedSlots (newTxOpps, *txOppsIt))
            {
              //copy the slot info into the new list
              newTxOpps.emplace_back (*txOppsIt);
            }
          //erase the selected one from the list
          txOppsIt = txOpps.erase (txOppsIt);
        }
    }

  //sort the list by SfnSf before returning
  newTxOpps.sort ();
  NS_ASSERT_MSG (newTxOpps.size () <= totalTx, "Number of randomly selected slots exceeded total number of TX");
  return newTxOpps;
}

} //namespace ns3
