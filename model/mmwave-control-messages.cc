/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include <ns3/log.h>
#include "mmwave-control-messages.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("mmWaveControlMessage");

MmWaveControlMessage::MmWaveControlMessage (void)
{
  NS_LOG_INFO (this);
}

MmWaveControlMessage::~MmWaveControlMessage (void)
{
  NS_LOG_INFO (this);
}

void
MmWaveControlMessage::SetMessageType (messageType type)
{
  m_messageType = type;
}

MmWaveControlMessage::messageType
MmWaveControlMessage::GetMessageType (void) const
{
  return m_messageType;
}

void
MmWaveControlMessage::SetSourceBwp (uint16_t bwpId)
{
  m_bwpId = bwpId;
}

uint16_t
MmWaveControlMessage::GetSourceBwp () const
{
  NS_ABORT_IF (m_bwpId < 0);
  return static_cast<uint16_t> (m_bwpId);
}

MmWaveSRMessage::MmWaveSRMessage ()
{
  NS_LOG_INFO (this);
  SetMessageType (MmWaveControlMessage::SR);
}

MmWaveSRMessage::~MmWaveSRMessage ()
{
  NS_LOG_INFO (this);
}

void
MmWaveSRMessage::SetRNTI (uint16_t rnti)
{
  m_rnti = rnti;
}

uint16_t
MmWaveSRMessage::GetRNTI () const
{
  return m_rnti;
}

MmWaveDlDciMessage::MmWaveDlDciMessage (const std::shared_ptr<DciInfoElementTdma> &dci)
  : m_dciInfoElement (dci)
{
  NS_LOG_INFO (this);
  SetMessageType (MmWaveControlMessage::DL_DCI);
}

MmWaveDlDciMessage::~MmWaveDlDciMessage (void)
{
  NS_LOG_INFO (this);
}

std::shared_ptr<DciInfoElementTdma>
MmWaveDlDciMessage::GetDciInfoElement (void)
{
  return m_dciInfoElement;
}

void
MmWaveDlDciMessage::SetKDelay (uint32_t delay)
{
  m_k = delay;
}

void
MmWaveDlDciMessage::SetK1Delay (uint32_t delay)
{
  m_k1 = delay;
}

uint32_t
MmWaveDlDciMessage::GetKDelay (void) const
{
  return m_k;
}

uint32_t
MmWaveDlDciMessage::GetK1Delay (void) const
{
  return m_k1;
}

MmWaveUlDciMessage::MmWaveUlDciMessage (const std::shared_ptr<DciInfoElementTdma> &dci)
  : m_dciInfoElement (dci)
{
  NS_LOG_INFO (this);
  SetMessageType (MmWaveControlMessage::UL_DCI);
}

MmWaveUlDciMessage::~MmWaveUlDciMessage (void)
{
  NS_LOG_INFO (this);
}

std::shared_ptr<DciInfoElementTdma>
MmWaveUlDciMessage::GetDciInfoElement (void)
{
  return m_dciInfoElement;
}

void
MmWaveUlDciMessage::SetKDelay (uint32_t delay)
{
  m_k = delay;
}

uint32_t
MmWaveUlDciMessage::GetKDelay (void) const
{
  return m_k;
}

MmWaveDlCqiMessage::MmWaveDlCqiMessage (void)
{
  SetMessageType (MmWaveControlMessage::DL_CQI);
  NS_LOG_INFO (this);
}
MmWaveDlCqiMessage::~MmWaveDlCqiMessage (void)
{
  NS_LOG_INFO (this);
}

void
MmWaveDlCqiMessage::SetDlCqi (DlCqiInfo cqi)
{
  m_cqi = cqi;
}

DlCqiInfo
MmWaveDlCqiMessage::GetDlCqi ()
{
  return m_cqi;
}

// ----------------------------------------------------------------------------------------------------------

MmWaveBsrMessage::MmWaveBsrMessage (void)
{
  SetMessageType (MmWaveControlMessage::BSR);
}


MmWaveBsrMessage::~MmWaveBsrMessage (void)
{

}

void
MmWaveBsrMessage::SetBsr (MacCeElement bsr)
{
  m_bsr = bsr;

}


MacCeElement
MmWaveBsrMessage::GetBsr (void)
{
  return m_bsr;
}

// ----------------------------------------------------------------------------------------------------------



MmWaveMibMessage::MmWaveMibMessage (void)
{
  SetMessageType (MmWaveControlMessage::MIB);
}


void
MmWaveMibMessage::SetMib (LteRrcSap::MasterInformationBlock  mib)
{
  m_mib = mib;
}

LteRrcSap::MasterInformationBlock
MmWaveMibMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



MmWaveSib1Message::MmWaveSib1Message (void)
{
  SetMessageType (MmWaveControlMessage::SIB1);
}


void
MmWaveSib1Message::SetSib1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_sib1 = sib1;
}

LteRrcSap::SystemInformationBlockType1
MmWaveSib1Message::GetSib1 () const
{
  return m_sib1;
}

// ----------------------------------------------------------------------------------------------------------

MmWaveRachPreambleMessage::MmWaveRachPreambleMessage (void)
{
  SetMessageType (MmWaveControlMessage::RACH_PREAMBLE);
}

MmWaveRachPreambleMessage::~MmWaveRachPreambleMessage()
{

}

void
MmWaveRachPreambleMessage::SetRapId (uint32_t rapId)
{
  m_rapId = rapId;
}

uint32_t
MmWaveRachPreambleMessage::GetRapId () const
{
  return m_rapId;
}

// ----------------------------------------------------------------------------------------------------------


MmWaveRarMessage::MmWaveRarMessage (void)
{
  SetMessageType (MmWaveControlMessage::RAR);
}

MmWaveRarMessage::~MmWaveRarMessage()
{

}

void
MmWaveRarMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t
MmWaveRarMessage::GetRaRnti () const
{
  return m_raRnti;
}

void
MmWaveRarMessage::AddRar (Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<MmWaveRarMessage::Rar>::const_iterator
MmWaveRarMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<MmWaveRarMessage::Rar>::const_iterator
MmWaveRarMessage::RarListEnd () const
{
  return m_rarList.end ();
}

MmWaveDlHarqFeedbackMessage::MmWaveDlHarqFeedbackMessage (void)
{
  SetMessageType (MmWaveControlMessage::DL_HARQ);
}


MmWaveDlHarqFeedbackMessage::~MmWaveDlHarqFeedbackMessage (void)
{

}

void
MmWaveDlHarqFeedbackMessage::SetDlHarqFeedback (DlHarqInfo m)
{
  m_dlHarqInfo = m;
}


DlHarqInfo
MmWaveDlHarqFeedbackMessage::GetDlHarqFeedback (void)
{
  return m_dlHarqInfo;
}

std::ostream &
operator<< (std::ostream &os, const LteNrTddSlotType &item)
{
  switch (item)
    {
    case LteNrTddSlotType::DL:
      os << "DL";
      break;
    case LteNrTddSlotType::F:
      os << "F";
      break;
    case LteNrTddSlotType::S:
      os << "S";
      break;
    case LteNrTddSlotType::UL:
      os << "UL";
      break;
    }
  return os;
}

}

