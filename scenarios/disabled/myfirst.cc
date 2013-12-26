/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS); //set time resolution to one nanosecond(default), can change only once.
  /* 
   * how to set logging level using env
   * $ export NS_LOG=UdpEchoClientApplication=level_all
   * $ export ’NS_LOG=UdpEchoClientApplication=level_all|prefix_func’
   * $ export ’NS_LOG=UdpEchoClientApplication=level_all|prefix_func:UdpEchoServerApplication=level_all|prefix_func’
   * $ export ’NS_LOG=*=level_all|prefix_func|prefix_time’
   * $ export NS_LOG=FirstScriptExample=info
   * $ export NS_LOG=MyLog=info
   */
  /*
   * in the source file
   * NS_LOG_COMPONENT_DEFINE ("MyLog"); //top of the file, same range can't duplicate.
   * LogComponentEnable ("MyLog", LOG_LEVEL_INFO); //same as set the env. Only need set one of them
   * NS_LOG_INFO ("Creating Topology");
   */
  /* 
   * procedure:
   * 	add in source: NS_LOG_COMPONENT_DEFINE, LogComponentEnable, NS_LOG_INFO
   * 	add env: export NS_LOG='xxx=info:xxx=level_all|prefix_func'
   */
  /*
   * the levels: 
   * 	LOG_ERROR	: serious
   * 	LOG_WARN
   * 	LOG_DEBUG
   * 	LOG_INFO	: general message
   * 	LOG_FUNCTION	: function tracing
   * 	LOG_LOGIC	: control flow tracing within functions
   * 	LOG_ALL
   * use in env:
   * 	level_all
   * 	prefix_func
   * 	prefix_time
   */
  NS_LOG_COMPONENT_DEFINE("MyLog");
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("MyLog", LOG_LEVEL_INFO);

  /*
   * command line
   * have to add this two line to use -- option.
   * 	CommandLine cmd;
   * 	cmd.Parse(argc, argv);
   * usage:
   * 	./waf --run "myfirst --PrintHelp"
   * 	./waf --run "myfirst --PrintAttributes=ns3::PointToPointNetDevice"
   *	/--------following only reset the default, so don't set again in program.-------/
   * 	./waf --run "scratch/myfirst
   * 		--ns3::PointToPointNetDevice::DataRate=5Mbps
   * 		--ns3::PointToPointChannel::Delay=2ms"	
   * 	./waf --run "myfirst --_num=2"
   */
  int _num=0;
  CommandLine cmd;
  cmd.AddValue("_num", "local num", _num);
  cmd.Parse(argc, argv);
  NS_LOG_DEBUG(_num);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes); //install NetDevice on each node in NodeContainer as well as the link between them

  InternetStackHelper stack;
  stack.Install (nodes); //install IP on each node in NodeContainer

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0"); //the first address is 10.1.1.1, the 2nd is 10.1.1.2, ...
  
  //perform actual address assignment
  //use interfaces to easily pull out addressing info later
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  UdpEchoServerHelper echoServer (9); //port number

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1)); //install application on nodes[1]
  serverApps.Start (Seconds (1.0)); //at 1.0s after Simulator::Run(), do this. The program maintain a list of things to do by time.
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9); //remote address, remote port number
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  /*
   * ASCII Trace
   * myfirst.tr:
   * + 
   * 2 
   * /NodeList/0/DeviceList/0/$ns3::PointToPointNetDevice/TxQueue/Enqueue
   * ns3::PppHeader ( 
   * 	Point-to-Point Protocol: IP (0x0021)) 
   * ns3::Ipv4Header (
   * 	tos 0x0 DSCP Default ECN Not-ECT ttl 64 id 0 protocol 17 offset (bytes) 0
   * 	flags [none] length: 1052 10.1.1.1 > 10.1.1.2) 
   * ns3::UdpHeader ( 
   * 	length:1032 49153 > 9) 
   * Payload (size=1024)
   *
   *************************************************************
   * +: An enqueue operation occurred on the device queue.
   * -:    dequeue
   * d: A packet was dropped, typically because the queue was full
   * r: 	     received
   */
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("myfirst.tr"));

  pointToPoint.EnablePcapAll("myfirst"); //filename is : <prefix>-<node_number>-<device_number>-<.pcap>

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
