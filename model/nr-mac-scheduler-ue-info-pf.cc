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

#include "nr-mac-scheduler-ue-info-pf.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerUeInfoPF");

/**
 * \brief Update DL PF metrics
 * \param timeWindow Time window to consider
 * \param config Config
 * \param amc AMC
 *
 * Updates m_currTputDl and m_avgTputDl by keeping in consideration
 * the assigned resources (in form of TBS) and the time window.
 */
void
NrMacSchedulerUeInfoPF::UpdateDlPFMetric (const NrMacSchedulerNs3::FTResources &totAssigned,
                                              double timeWindow,
                                              const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);

  NrMacSchedulerUeInfo::UpdateDlMetric (amc);

  m_currTputDl = static_cast<double> (m_dlTbSize) / (totAssigned.m_sym);
  m_avgTputDl = ((1.0 - (1.0 / static_cast<double> (timeWindow))) * m_lastAvgTputDl) +
    ((1.0 / timeWindow) * m_currTputDl);

  NS_LOG_DEBUG ("Update DL PF Metric for UE " << static_cast<uint32_t> (m_rnti) <<
                " DL TBS: " << m_dlTbSize << " Updated currTputDl " <<
                m_currTputDl << " avgTputDl " << m_avgTputDl << " over n. of syms: " <<
                +totAssigned.m_sym <<  ", last Avg TH Dl" << m_lastAvgTputDl <<
                " total sym assigned " << static_cast<uint32_t> (totAssigned.m_sym) <<
                " updated DL metric: " << m_potentialTput / std::max (1E-9, m_avgTputDl));
}

/**
 * \brief Update UL PF metrics
 * \param timeWindow Time window to consider
 * \param config Config
 * \param amc AMC
 *
 * Updates m_currTputUl and m_avgTputUl by keeping in consideration
 * the assigned resources (in form of TBS) and the time window.
 */
void
NrMacSchedulerUeInfoPF::UpdateUlPFMetric (const NrMacSchedulerNs3::FTResources &totAssigned,
                                              double timeWindow,
                                              const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);

  NrMacSchedulerUeInfo::UpdateUlMetric (amc);

  m_currTputUl = static_cast<double> (m_ulTbSize) / (totAssigned.m_sym);
  m_avgTputUl = ((1.0 - (1.0 / static_cast<double> (timeWindow))) * m_lastAvgTputUl) +
    ((1.0 / timeWindow) * m_currTputUl);

  NS_LOG_DEBUG ("Update UL PF Metric for UE " << static_cast<uint32_t> (m_rnti) <<
                " UL TBS: " << m_ulTbSize << " Updated currTputUl " <<
                m_currTputUl << " avgTputUl " << m_avgTputUl << " over n. of syms: " <<
                +totAssigned.m_sym <<  ", last Avg TH Ul" << m_lastAvgTputUl <<
                " total sym assigned " << static_cast<uint32_t> (totAssigned.m_sym) <<
                " updated UL metric: " << m_potentialTputUl / std::max (1E-9, m_avgTputUl));
}

void
NrMacSchedulerUeInfoPF::CalculatePotentialTPut (const NrMacSchedulerNs3::FTResources &assignableInIteration,
                                                    const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);

  uint32_t rbsAssignable = assignableInIteration.m_rbg * GetNumRbPerRbg ();
  m_potentialTput =  amc->CalculateTbSize (m_dlMcs, rbsAssignable);
  m_potentialTput /= assignableInIteration.m_sym;

  NS_LOG_INFO ("UE " << m_rnti << " potentialTputDl " << m_potentialTput <<
               " lastAvgThDl " << m_lastAvgTputDl <<
               " DL metric: " << m_potentialTput / std::max (1E-9, m_avgTputDl));
}

void
NrMacSchedulerUeInfoPF::CalculatePotentialTPutUl (const NrMacSchedulerNs3::FTResources &assignableInIteration,
                                                    const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);

  uint32_t rbsAssignable = assignableInIteration.m_rbg * GetNumRbPerRbg ();
  m_potentialTputUl =  amc->CalculateTbSize (m_ulMcs, rbsAssignable);
  m_potentialTputUl /= assignableInIteration.m_sym;

  NS_LOG_INFO ("UE " << m_rnti << " potentialTputUl " << m_potentialTputUl <<
               " lastAvgThUl " << m_lastAvgTputUl <<
               " UL metric: " << m_potentialTputUl / std::max (1E-9, m_avgTputUl));
}

} // namespace ns3
