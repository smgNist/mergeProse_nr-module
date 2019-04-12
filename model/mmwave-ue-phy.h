/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
*   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef MMWAVE_UE_PHY_H
#define MMWAVE_UE_PHY_H

#include <ns3/mmwave-phy.h>
#include "nr-amc.h"
#include <ns3/lte-ue-phy-sap.h>
#include <ns3/lte-ue-cphy-sap.h>
#include <ns3/mmwave-harq-phy.h>
#include <ns3/antenna-array-3gpp-model.h>

namespace ns3 {

class MmWaveUePhy : public MmWavePhy
{
  friend class UeMemberLteUePhySapProvider;
  friend class MemberLteUeCphySapProvider<MmWaveUePhy>;

public:
  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoInitialize (void) override; // Public because it's called by hand,
                                             // and not by aggregation, in MmWaveNetDevice

  /**
   * \brief MmWaveUePhy default constructor. Is there for ns-3 object system, but should not be used.
   */
  MmWaveUePhy ();

  /**
   * \brief MmWaveUePhy real constructor
   * \param dlPhy DL spectrum phy
   * \param ulPhy UL spectrum phy
   * \param n Pointer to the node owning this instance
   *
   * Usually called by the helper. It starts the event loop for the ue.
   */
  MmWaveUePhy (Ptr<MmWaveSpectrumPhy> dlPhy, Ptr<MmWaveSpectrumPhy> ulPhy, const Ptr<Node> &n);

  /**
   * \brief ~MmWaveUePhy
   */
  virtual ~MmWaveUePhy () override;

  /**
   * \brief Retrieve the pointer for the C PHY SAP provider (AKA the PHY interface towards the RRC)
   * \return the C PHY SAP pointer
   */
  LteUeCphySapProvider* GetUeCphySapProvider () __attribute__((warn_unused_result));

  /**
   * \brief Install ue C PHY SAP user (AKA the PHY interface towards the RRC)
   * \param s the C PHY SAP user pointer to install
   */
  void SetUeCphySapUser (LteUeCphySapUser* s);

  /**
   * \brief Install the PHY sap user (AKA the UE MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (MmWaveUePhySapUser* ptr);

  /**
   * \brief Set the transmission power for the UE
   *
   * Please note that there is also an attribute ("MmWaveUePhy::TxPower")
   * \param pow power
   */
  void SetTxPower (double pow);

  /**
   * \brief Retrieve the TX power of the UE
   *
   * Please note that there is also an attribute ("MmWaveUePhy::TxPower")
   * \return the TX power of the UE
   */
  double GetTxPower () const __attribute__((warn_unused_result));

  /**
   * \brief Register the UE to a certain Enb
   *
   * Install the configuration parameters in the UE. At the moment, the code
   * does not reconfigure itself when the PhyMacCommon parameters change,
   * so you can call this function only one (therefore, no handoff)
   *
   * \param cellId the CELL ID of the ENB
   * \param config the ENB configuration
   */
  void RegisterToEnb (uint16_t cellId, Ptr<MmWavePhyMacCommon> config);

  /**
   * \brief Retrieve the DlSpectrumPhy pointer
   *
   * As this function is used mainly to get traced values out of DlSpectrum,
   * it should be removed and the traces connected (and redirected) here.
   * \return A pointer to the DlSpectrumPhy of this UE
   */
  virtual Ptr<MmWaveSpectrumPhy> GetDlSpectrumPhy () const override __attribute__((warn_unused_result));

  /**
   * \brief Receive a list of CTRL messages
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param msgList message list
   */
  void PhyCtrlMessagesReceived (const std::list<Ptr<MmWaveControlMessage> > &msgList);

  /**
   * \brief Receive a PHY data packet
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param p Received packet
   */
  void PhyDataPacketReceived (const Ptr<Packet> &p);

  /**
   * \brief Generate a DL CQI report
   *
   * Connected by the helper to a callback in mmWaveChunkProcessor.
   *
   * \param sinr the SINR
   */
  void GenerateDlCqiReport (const SpectrumValue& sinr);

  /**
   * \brief Get the current RNTI of the user
   *
   * \return the current RNTI of the user
   */
  uint16_t GetRnti () __attribute__((warn_unused_result));

  /**
   * \brief Receive the HARQ feedback on the transmission
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param m the HARQ feedback
   */
  void ReceiveLteDlHarqFeedback (const DlHarqInfo &m);

  // From mmwave phy. Not used in the UE
  virtual AntennaArrayModel::BeamId GetBeamId (uint16_t rnti) const override;

protected:
  // From object
  virtual void DoDispose (void) override;

private:

  /**
   * \brief Create a DlCqiFeedback message
   * \param sinr the SINR value
   * \return a CTRL message with the CQI feedback
   */
  Ptr<MmWaveDlCqiMessage> CreateDlCqiFeedbackMessage (const SpectrumValue& sinr) __attribute__((warn_unused_result));
  /**
   * \brief Receive DL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL CTRL will end
   */
  Time DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Transmit UL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL CTRL will end
   */
  Time UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Receive DL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL data will end
   */
  Time DlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Transmit UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL data will end
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  void StartSlot (uint16_t frameNum, uint8_t subframeNum, uint16_t slotNum);
  void StartVarTti ();
  void EndVarTti ();
  void SetSubChannelsForTransmission (std::vector <int> mask);
  void DoSendControlMessage (Ptr<MmWaveControlMessage> msg);
  void SendDataChannels (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time duration, uint8_t slotInd);
  void SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time prd);

  // SAP methods
  void DoReset ();
  void DoStartCellSearch (uint16_t dlEarfcn);
  void DoSynchronizeWithEnb (uint16_t cellId);
  void DoSynchronizeWithEnb (uint16_t cellId, uint16_t dlEarfcn);
  void DoSetPa (double pa);
  /**
   * \param rsrpFilterCoefficient value. Determines the strength of
   * smoothing effect induced by layer 3 filtering of RSRP
   * used for uplink power control in all attached UE.
   * If equals to 0, no layer 3 filtering is applicable.
   */
  void DoSetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient);
  void DoSetDlBandwidth (uint8_t ulBandwidth);
  void DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth);
  void DoConfigureReferenceSignalPower (int8_t referenceSignalPower);
  void DoSetRnti (uint16_t rnti);
  void DoSetTransmissionMode (uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t srcCi);

private:
  MmWaveUePhySapUser* m_phySapUser;             //!< SAP pointer
  LteUeCphySapProvider* m_ueCphySapProvider;    //!< SAP pointer
  LteUeCphySapUser* m_ueCphySapUser;            //!< SAP pointer

  Ptr<NrAmc> m_amc;  //!< AMC model used to compute the CQI feedback

  Time m_wbCqiPeriod;       /**< Wideband Periodic CQI: 2, 5, 10, 16, 20, 32, 40, 64, 80 or 160 ms */
  Time m_wbCqiLast;
  Time m_lastSlotStart; //!< Time of the last slot start

  bool m_ulConfigured {false};     //!< Flag to indicate if RRC configured the UL
  bool m_receptionEnabled {false}; //!< Flag to indicate if we are currently receiveing data
  uint16_t m_rnti {0};             //!< Current RNTI of the user
  uint32_t m_currTbs {0};          //!< Current TBS of the receiveing DL data (used to compute the feedback)

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_reportCurrentCellRsrpSinrTrace; //!< Report the rsrp
  TracedCallback<uint64_t, uint64_t> m_reportUlTbSize; //!< Report the UL TBS
  TracedCallback<uint64_t, uint64_t> m_reportDlTbSize; //!< Report the DL TBS
};

}

#endif /* MMWAVE_UE_PHY_H */
