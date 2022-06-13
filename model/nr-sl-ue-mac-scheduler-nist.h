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

#ifndef NR_SL_UE_MAC_SCHEDULER_NIST_H
#define NR_SL_UE_MAC_SCHEDULER_NIST_H


#include "nr-sl-ue-mac-scheduler-ns3.h"
#include "nr-sl-phy-mac-common.h"

namespace ns3 {

/**
 * \ingroup scheduler
 *
 * \brief An extension of NrSlUeMacSchedulerSimple scheduler to support the
 *        allocation of multiple logical channels per destination
 */
class NrSlUeMacSchedulerNist : public NrSlUeMacSchedulerNs3
{
public:
  /**
   * \brief GetTypeId
   *
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacSchedulerNs3 default constructor
   */
  NrSlUeMacSchedulerNist ();

  /**
   * \brief Do the NR Sidelink allocation
   *
   * This method performs the allocation of resources.  This scheduler serves
   * the LCs by priority, and then by creation time (insertion order) among
   * the LCs with the same priority
   *
   * \param txOpps The list of the txOpps from the UE MAC
   * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
   *        for which UE MAC asked the scheduler to allocate the resources
   * \param slotAllocList The slot allocation list to be updated by this scheduler
   * \return The status of the allocation, true if the destination has been
   *         allocated some resources; false otherwise.
   */
  virtual bool DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                 const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                 std::set<NrSlSlotAlloc> &slotAllocList) override;

private:
  struct SbChInfo
  {
    uint8_t numSubCh {
      0
    };                    //!< The minimum number of contiguous subchannels that could be used for each slot.
    std::vector <std::vector<uint8_t> > availSbChIndPerSlot; //!< The vector containing the available subchannel index for each slot
  };
  /**
   * \brief Select the slots randomly from the available slots
   *
   * \param txOpps The list of the available TX opportunities
   * \return the set containing the indices of the randomly chosen slots in the
   *         txOpps list
   */
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
  /**
   * \brief Randomly select the number of slots from the slots given by UE MAC
   *
   * If K denotes the total number of available slots, and N_PSSCH_maxTx is the
   * maximum number of PSSCH configured transmissions, then:
   *
   * N_Selected = N_PSSCH_maxTx , if K >= N_PSSCH_maxTx
   * otherwise;
   * N_Selected = K
   *
   * \param txOpps The list of the available slots
   * \return The list of randomly selected slots
   */
  RandomlySelectSlots (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
  /**
   * \brief Get available subchannel information
   *
   * This method takes as input the randomly selected slots and computes the
   * maximum number of contiguous subchannels that are available for all
   * those slots. Moreover, it also returns the indexes of the available
   * subchannels for each slot.
   *
   * \param txOpps The list of randomly selected slots
   * \return A struct object of type SbChInfo
   */
  SbChInfo GetAvailSbChInfo (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
  /**
   * \brief Randomly select the starting subchannel index
   *
   * This method, for each slot randomly selects the starting subchannel
   * index by taking into account the number of available contiguous subchannels
   * and the number of subchannels that needs to be assigned.
   *
   * \param sbChInfo a struct object of type SbChInfo
   * \param assignedSbCh The number of assigned subchannels
   * \return A vector containing the randomly chosen starting subchannel index
   *         for each slot.
   */
  std::vector <uint8_t> RandSelSbChStart (SbChInfo sbChInfo, uint8_t assignedSbCh);
};

} //namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_NIST_H */
