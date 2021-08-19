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


#include "nr-sl-ue-mac-scheduler-nist.h"

#include <ns3/log.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerNist");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerNist);

NrSlUeMacSchedulerNist::NrSlUeMacSchedulerNist ()
{}

TypeId
NrSlUeMacSchedulerNist::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerNist")
    .SetParent<NrSlUeMacSchedulerNs3> ()
    .AddConstructor<NrSlUeMacSchedulerNist> ()
    .SetGroupName ("nr")
  ;
  return tid;
}

bool
NrSlUeMacSchedulerNist::DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                          const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                          std::set<NrSlSlotAlloc> &slotAllocList)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;
  NS_ASSERT_MSG (txOpps.size () > 0, "Scheduler received an empty txOpps list from UE MAC");
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy


  //1. Calculate total buffer size (all LCGs) and the lowest priority value (corresponding to the highest priority)
  //   among the LCs with data to transmit
  uint32_t totalBufferSize = 0;
  uint8_t lowestPrioValue = std::numeric_limits <uint8_t>::max ();
  std::set<uint8_t> priosWithData;
  NS_LOG_DEBUG ("dstL2Id " << dstInfo->GetDstL2Id () << " has " << lcgMap.size () << " LCGs:");
  for (const auto &itLcg:lcgMap)
    {
      NS_LOG_DEBUG ( "-LCG " << (uint16_t) itLcg.second->m_id << " has " << itLcg.second->GetNumOfLC () << " LCs:");
      std::vector<uint8_t> lcIdsVector = itLcg.second->GetLCId (); //Vector with LC Ids of this LCG

      for (const auto &itLcId:lcIdsVector)
        {
          NS_LOG_DEBUG ( " -LC " << (uint16_t) itLcId  << " has " << itLcg.second->GetTotalSizeOfLC (itLcId)
                                 << " bytes to transmit and priority value = " << (uint16_t) itLcg.second->GetLcPriority (itLcId));
          if (itLcg.second->GetTotalSizeOfLC (itLcId) > 0)
            {
              totalBufferSize += itLcg.second->GetTotalSizeOfLC (itLcId);
              priosWithData.insert (itLcg.second->GetLcPriority (itLcId)); // remember: sets do not insert duplicates
              if (itLcg.second->GetLcPriority (itLcId) < lowestPrioValue)
                {
                  lowestPrioValue = itLcg.second->GetLcPriority (itLcId);
                }
            }
        }
    }
  NS_LOG_DEBUG ("Total buffer size = " << totalBufferSize << " lowest priority value (higher priority) with data to transmit = " << (uint16_t) lowestPrioValue);


  if (totalBufferSize == 0)
    {
      return allocated;
    }

  NS_ASSERT_MSG (IsNrSlMcsFixed (), "Attribute FixNrSlMcs must be true for NrSlUeMacSchedulerNist scheduler");


  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> selectedTxOpps;
  selectedTxOpps = RandomlySelectSlots (txOpps);
  NS_ASSERT_MSG (selectedTxOpps.size () > 0, "Scheduler should select at least 1 slot from txOpps");
  NS_LOG_DEBUG ("Number of tx Opps = " << selectedTxOpps.size ());


  uint32_t tbs = 0;
  uint8_t assignedSbCh = 0;

  uint16_t availableSymbols = selectedTxOpps.begin ()->slPsschSymLength;
  uint16_t sbChSize = selectedTxOpps.begin ()->slSubchannelSize;
  NS_LOG_DEBUG ("Total available symbols for PSSCH = " << availableSymbols);


  //2. Calculate the smaller TB in which the total buffer can fit, if possible
  auto sbChInfo = GetAvailSbChInfo (selectedTxOpps);
  NS_ABORT_MSG_IF (sbChInfo.availSbChIndPerSlot.size () != selectedTxOpps.size (), "subChInfo vector does not have info for all the selected slots");
  do
    {
      assignedSbCh++;
      tbs = GetNrSlAmc ()->CalculateTbSize (dstInfo->GetDstMcs (), sbChSize * assignedSbCh * availableSymbols);
    }
  while (tbs < totalBufferSize + 5 /*(5 bytes overhead of SCI format 2A)*/ && (sbChInfo.numSubCh - assignedSbCh) > 0);

  NS_LOG_DEBUG ("Number of allocated subchannles = " << +assignedSbCh);

  //TBS available for data:
  tbs = tbs - 5 /*(5 bytes overhead of SCI stage 2)*/;
  NS_LOG_DEBUG ("TBS = " << tbs);


  //3. Obtain the LCs that are going to be served with this allocation. The
  //   logic below serve the LCs by priority, and by creation time among the
  //   LCs with the same priority (insertion order in the 'lcgMap' structure)
  uint32_t servedBuffer = 0;
  bool full = false;
  std::vector <SlRlcPduInfo> tmpLcsToServe; //!< The vector containing the transport block size per LC id

  for (const auto &itPrio: priosWithData) //priosWithData is an ordered set
    {

      for (const auto &itLcg:lcgMap)
        {
          std::vector<uint8_t> lcIdsVector = itLcg.second->GetLCId (); //Vector with LC Ids of this LCG
          for (const auto &itLcId:lcIdsVector)
            {
              if (itLcg.second->GetLcPriority (itLcId) == itPrio
                  && itLcg.second->GetTotalSizeOfLC (itLcId) > 0)
                {
                  uint32_t toServe = 0;
                  if (servedBuffer + itLcg.second->GetTotalSizeOfLC (itLcId) <= tbs)
                    {
                      toServe = itLcg.second->GetTotalSizeOfLC (itLcId);
                    }
                  else
                    {
                      toServe = tbs - itLcg.second->GetTotalSizeOfLC (itLcId);
                      full = true;
                    }
                  if (toServe > 0)
                    {
                      servedBuffer = +toServe;
                      SlRlcPduInfo slRlcPduInfo (itLcId, toServe);
                      tmpLcsToServe.push_back (slRlcPduInfo);
                      NS_LOG_DEBUG ("LCG " << (uint16_t) itLcg.second->m_id <<
                                    " LC " << (uint16_t) itLcId  <<
                                    " priority value " << (uint32_t)itPrio <<
                                    " has " << toServe << " bytes assigned in the TB");
                      //Update LC buffer
                      itLcg.second->AssignedData (itLcId, toServe);

                    }
                  if (full)
                    {
                      break;
                    }
                }
            }
          if (full)
            {
              break;
            }
        }

    }
  NS_LOG_DEBUG ("Served Buffer (bytes) = " << servedBuffer << " Number of served LCs = " << tmpLcsToServe.size ());


  std::vector <uint8_t> startSubChIndexPerSlot = RandSelSbChStart (sbChInfo, assignedSbCh);

  allocated = true;

  auto itsbChIndexPerSlot = startSubChIndexPerSlot.cbegin ();
  auto itTxOpps = selectedTxOpps.cbegin ();


  for (; itTxOpps != selectedTxOpps.cend () && itsbChIndexPerSlot != startSubChIndexPerSlot.cend (); ++itTxOpps, ++itsbChIndexPerSlot)
    {
      NS_LOG_DEBUG ("itTxOpps->sfn = " << itTxOpps->sfn);

      NrSlSlotAlloc slotAlloc;
      slotAlloc.sfn = itTxOpps->sfn;
      slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
      slotAlloc.priority = lowestPrioValue; //Use higher priority among LCs
      slotAlloc.slRlcPduInfo.insert (slotAlloc.slRlcPduInfo.end (), tmpLcsToServe.begin (), tmpLcsToServe.end ());
      slotAlloc.mcs = dstInfo->GetDstMcs ();
      //PSCCH
      slotAlloc.numSlPscchRbs = itTxOpps->numSlPscchRbs;
      slotAlloc.slPscchSymStart = itTxOpps->slPscchSymStart;
      slotAlloc.slPscchSymLength = itTxOpps->slPscchSymLength;
      //PSSCH
      slotAlloc.slPsschSymStart = itTxOpps->slPsschSymStart;
      slotAlloc.slPsschSymLength = availableSymbols;
      slotAlloc.slPsschSubChStart = *itsbChIndexPerSlot;
      slotAlloc.slPsschSubChLength = assignedSbCh;
      slotAlloc.maxNumPerReserve = itTxOpps->slMaxNumPerReserve;
      slotAlloc.ndi = slotAllocList.empty () == true ? 1 : 0;
      slotAlloc.rv = GetRv (static_cast<uint8_t> (slotAllocList.size ()));
      if (static_cast<uint16_t> (slotAllocList.size ()) % itTxOpps->slMaxNumPerReserve == 0)
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


std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrSlUeMacSchedulerNist::RandomlySelectSlots (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this);

  uint8_t totalTx = GetSlMaxTxTransNumPssch ();
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> newTxOpps;

  if (txOpps.size () > totalTx)
    {
      while (newTxOpps.size () != totalTx)
        {
          auto txOppsIt = txOpps.begin ();
          // Walk through list until the random element is reached
          std::advance (txOppsIt, m_uniformVariable->GetInteger (0, txOpps.size () - 1));
          //copy the randomly selected slot info into the new list
          newTxOpps.emplace_back (*txOppsIt);
          //erase the selected one from the list
          txOppsIt = txOpps.erase (txOppsIt);
        }
    }
  else
    {
      newTxOpps = txOpps;
    }
  //sort the list by SfnSf before returning
  newTxOpps.sort ();
  NS_ASSERT_MSG (newTxOpps.size () <= totalTx, "Number of randomly selected slots exceeded total number of TX");
  return newTxOpps;
}

NrSlUeMacSchedulerNist::SbChInfo
NrSlUeMacSchedulerNist::GetAvailSbChInfo (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this << txOpps.size ());
  //txOpps are the randomly selected slots for 1st Tx and possible ReTx
  SbChInfo info;
  info.numSubCh = GetTotalSubCh ();
  std::vector <std::vector<uint8_t> > availSbChIndPerSlot;
  for (const auto &it:txOpps)
    {
      std::vector<uint8_t> indexes;
      for (uint8_t i = 0; i < GetTotalSubCh (); i++)
        {
          auto it2 = it.occupiedSbCh.find (i);
          if (it2 == it.occupiedSbCh.end ())
            {
              //available subchannel index(s)
              indexes.push_back (i);
            }
        }
      //it may happen that all sub-channels are occupied
      //remember scheduler can get a slot with all the
      //subchannels occupied because of 3 dB RSRP threshold
      //at UE MAC
      if (indexes.size () == 0)
        {
          for (uint8_t i = 0; i < GetTotalSubCh (); i++)
            {
              indexes.push_back (i);
            }
        }

      NS_ABORT_MSG_IF (indexes.size () == 0, "Available subchannels are zero");

      availSbChIndPerSlot.push_back (indexes);
      uint8_t counter = 0;
      for (uint8_t i = 0; i < indexes.size (); i++)
        {
          uint8_t counter2 = 0;
          uint8_t j = i;
          do
            {
              j++;
              if (j != indexes.size ())
                {
                  counter2++;
                }
              else
                {
                  counter2++;
                  break;
                }
            }
          while (indexes.at (j) == indexes.at (j - 1) + 1);

          counter = std::max (counter, counter2);
        }

      info.numSubCh = std::min (counter, info.numSubCh);
    }

  info.availSbChIndPerSlot = availSbChIndPerSlot;
  for (const auto &it:info.availSbChIndPerSlot)
    {
      NS_ABORT_MSG_IF (it.size () == 0, "Available subchannel size is 0");
    }
  return info;
}

std::vector <uint8_t>
NrSlUeMacSchedulerNist::RandSelSbChStart (SbChInfo sbChInfo, uint8_t assignedSbCh)
{
  NS_LOG_FUNCTION (this << +sbChInfo.numSubCh << sbChInfo.availSbChIndPerSlot.size () << +assignedSbCh);

  std::vector <uint8_t> subChInStartPerSlot;
  uint8_t minContgSbCh = sbChInfo.numSubCh;

  for (const auto &it:sbChInfo.availSbChIndPerSlot)
    {
      if (minContgSbCh == GetTotalSubCh () && assignedSbCh == 1)
        {
          //quick exit
          uint8_t randIndex = static_cast<uint8_t> (m_uniformVariable->GetInteger (0, GetTotalSubCh () - 1));
          subChInStartPerSlot.push_back (randIndex);
        }
      else
        {
          bool foundRandSbChStart = false;
          auto indexes = it;
          do
            {
              NS_ABORT_MSG_IF (indexes.size () == 0, "No subchannels available to choose from");
              uint8_t randIndex = static_cast<uint8_t> (m_uniformVariable->GetInteger (0, indexes.size () - 1));
              NS_LOG_DEBUG ("Randomly drawn index of the subchannel vector is " << +randIndex);
              uint8_t sbChCounter = 0;
              for (uint8_t i = randIndex; i < indexes.size (); i++)
                {
                  sbChCounter++;
                  auto it = std::find (indexes.begin (), indexes.end (), indexes.at (i) + 1);
                  if (sbChCounter == assignedSbCh)
                    {
                      foundRandSbChStart = true;
                      NS_LOG_DEBUG ("Random starting sbch is " << +indexes.at (randIndex));
                      subChInStartPerSlot.push_back (indexes.at (randIndex));
                      break;
                    }
                  if (it == indexes.end ())
                    {
                      break;
                    }
                }
            }
          while (foundRandSbChStart == false);
        }
    }

  return subChInStartPerSlot;
}


} //namespace ns3
