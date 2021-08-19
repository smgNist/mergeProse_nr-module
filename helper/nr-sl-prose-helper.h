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


#ifndef NR_SL_PROSE_HELPER_H
#define NR_SL_PROSE_HELPER_H

#include <ns3/object.h>
#include <ns3/net-device-container.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {

class NrUeNetDevice;


class NrSlProseHelper : public Object
{

/**
 * \brief Class to help in the configuration of the Proximity Service (ProSe)
 *        functionalities
 */
public:
  /**
   * \brief Constructor
   */
  NrSlProseHelper (void);
  /**
   * \brief Destructor
   */
  virtual ~NrSlProseHelper (void);
  /**
   * \brief GetTypeId, inherited from Object
   *
   * \returns The TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Prepare UE for ProSe
   *
   * This method installs the ProSe layer in the UE(s) in the container
   *
   * \param c The NetDeviceContainer
   */
  void PrepareUesForProse (NetDeviceContainer c);
  /**
   * \brief Prepare UE for Unicast ProSe Direct Communication
   *
   * This method configures the UE(s) in the container to be able to do unicast
   * ProSe direct communication
   *
   * \param c The \c NetDeviceContainer
   */
  void PrepareUesForUnicast (NetDeviceContainer c);
  /**
   * \brief Establish a 5G ProSe direct link between the two UEs using an ideal protocol
   *
   * This method schedules the creation of the direct link instances in both UEs
   * participating in the direct link. Then, the ProSe layer configures the direct
   * link instances and starts the establishment procedure in the initiating UE.
   * An ideal protocol means that PC5-S messages used for establishing and
   * maintaining the direct link connection are delivered directly between
   * direct link instances, as opposed to going through the protocol stack and
   * being sent over the SL.
   *
   * \param time The time at which the direct link instances should be created
   * \param initUe The UE initiating the establishment procedure
   * \param initUeIp The IP address used by the initiating UE
   * \param trgtUE The peer UE
   * \param trgtUeIp The IP address used by the target UE
   */
  void EstablishIdealDirectLink (Time time, Ptr<NetDevice> initUe, Ipv4Address initUeIp, Ptr<NetDevice> trgtUe, Ipv4Address trgtUeIp);
  /**
   * \brief Establish a 5G ProSe direct link between the two UEs using the real
   *        protocol
   *
   * This method schedules the creation of the direct link instances in both UEs
   * participating in the direct link. Then, the ProSe layer configures the direct
   * link instances and starts the establishment procedure in the initiating UE.
   * A real protocol means that PC5-S messages used for establishing and
   * maintaining the direct link connection goes through the protocol stack,
   * are transmitted in SL-SRBs and sent over the SL
   *
   * \param time The time at which the direct link instances should be created
   * \param initUe The UE initiating the establishment procedure
   * \param initUeIp The IP address used by the initiating UE
   * \param trgtUE The peer UE
   * \param trgtUeIp The IP address used by the target UE
   */
  void EstablishRealDirectLink (Time time, Ptr<NetDevice> initUe, Ipv4Address initUeIp, Ptr<NetDevice> trgtUe, Ipv4Address trgtUeIp);

protected:
  /**
   * \brief \c DoDispose method inherited from \c Object
   */
  virtual void DoDispose (void) override;

private:
  /**
  * \brief Prepare Single UE for ProSe
  *
  *  Install ProSe layer in the device and connect corresponding SAPs
  *
  * \param NrUeDev The Ptr to NR UE NetDevice
  */
  void PrepareSingleUeForProse (Ptr<NrUeNetDevice> nrUeDev);
  /**
  * \brief  Prepare UE for Unicast ProSe Direct Communication
  *
  * \param NrUeDev The Ptr to NR UE NetDevice
  */
  void PrepareSingleUeForUnicast (Ptr<NrUeNetDevice> nrUeDev);
};

}

#endif /* NR_SL_PROSE_HELPER_H */

