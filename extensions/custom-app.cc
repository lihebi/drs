/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

// custom-app.cc

#include "custom-app.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("CustomApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CustomApp);

// register NS-3 type
TypeId
CustomApp::GetTypeId ()
{
  static TypeId tid = TypeId ("CustomApp")
    .SetParent<ndn::App> ()
    .AddConstructor<CustomApp> ()
    ;
  return tid;
}

// Processing upon start of the application
void
CustomApp::StartApplication ()
{
  ndn::App::StartApplication ();
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/prefix"); // now prefix contains ``/``
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
  fib->Add (*prefix, m_face, 0);
  Simulator::Schedule (Seconds (1.0), &CustomApp::SendInterest, this);
}
void
CustomApp::StopApplication ()
{
  ndn::App::StopApplication ();
}

void
CustomApp::SendInterest ()
{
  Ptr<ndn::Name> prefix = Create<ndn::Name> ("/prefix"); // another way to create name
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));
  NS_LOG_DEBUG ("Sending Interest packet for " << *prefix);
  m_transmittedInterests (interest, this, m_face);
  m_face->ReceiveInterest (interest);
}

// Callback that will be called when Interest arrives
void
CustomApp::OnInterest (Ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);
  
  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());
}

// Callback that will be called when Data arrives
void
CustomApp::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << contentObject->GetName ());

  std::cout << "DATA received for name " << contentObject->GetName () << std::endl;
}

} // namespace ns3
