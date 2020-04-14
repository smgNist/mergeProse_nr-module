/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#include "bandwidth-part-ue.h"

#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/mmwave-ue-phy.h>
#include <ns3/mmwave-ue-mac.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BandwidthPartUe");

NS_OBJECT_ENSURE_REGISTERED (BandwidthPartUe);

TypeId BandwidthPartUe::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::BandwidthPartUe")
    .SetParent<ComponentCarrier> ()
    .AddConstructor<BandwidthPartUe> ()
    .AddAttribute ("MmWaveUePhy",
                   "The PHY associated to this BandwidthPartUe",
                   PointerValue (),
                   MakePointerAccessor (&BandwidthPartUe::m_phy),
                   MakePointerChecker <MmWaveUePhy> ())
    .AddAttribute ("MmWaveUeMac",
                   "The MAC associated to this BandwidthPartUe",
                   PointerValue (),
                   MakePointerAccessor (&BandwidthPartUe::m_mac),
                   MakePointerChecker <MmWaveUeMac> ())
  ;
  return tid;
}
BandwidthPartUe::BandwidthPartUe ()
  : ComponentCarrier()
{
  NS_LOG_FUNCTION (this);
  m_phy = nullptr;
}

BandwidthPartUe::~BandwidthPartUe (void)
{
  NS_LOG_FUNCTION (this);
}

void
BandwidthPartUe::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phy->Dispose ();
  m_phy = nullptr;
  m_mac->Dispose ();
  m_mac = nullptr;
  Object::DoDispose ();
}

void
BandwidthPartUe::SetPhy (Ptr<MmWaveUePhy> s)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_phy != nullptr);
  m_phy = s;
}


Ptr<MmWaveUePhy>
BandwidthPartUe::GetPhy () const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
BandwidthPartUe::SetMac (Ptr<MmWaveUeMac> s)
{
  NS_LOG_FUNCTION (this);
  m_mac = s;
}

Ptr<MmWaveUeMac>
BandwidthPartUe::GetMac () const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

} // namespace ns3

