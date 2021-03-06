/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
 /*
  *  * Configuration:
  * 1. TCP payload size = 1460 bytes - 1 packet per second
  * 2. Network RTT ~= 10 ms
  * 3. delayed ACK is disabled for TCP
  * 4. Short GI has been enabled for all nodes - only applicable for 802.11n
  * 5. Beacon: Long preamble, 802.11b HR/DSSS - 1 Mbps
  * 6. TCP Tx - 802.11n 72.2 Mbps - BW = 20 MHz, Short GI
  * 7. TCP ACK Rx - 802.11b HR/DSSS 1 Mbps
  * Copyright (c) 2015, IMDEA Networks Institute
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
  * Author: Hany Assasa <hany.assasa@gmail.com>
 .*
  * This is a simple example to test TCP over 802.11n (with MPDU aggregation enabled).
  *
  * Network topology:
  *
  *   Ap    STA
  *   *      *
  *   |      |
  *   n1     n2
  *
  * In this example, an HT station sends TCP packets to the access point.
  * We report the total throughput received during a window of 100ms.
  * The user can specify the application data rate and choose the variant
  * of TCP i.e. congestion control algorithm to use.
  */
 
 #include "ns3/command-line.h"
 #include "ns3/config.h"
 #include "ns3/string.h"
 #include "ns3/log.h"
 #include "ns3/yans-wifi-helper.h"
 #include "ns3/ssid.h"
 #include "ns3/mobility-helper.h"
 #include "ns3/on-off-helper.h"
 #include "ns3/yans-wifi-channel.h"
 #include "ns3/mobility-model.h"
 #include "ns3/packet-sink.h"
 #include "ns3/packet-sink-helper.h"
 #include "ns3/tcp-westwood.h"
 #include "ns3/internet-stack-helper.h"
 #include "ns3/ipv4-address-helper.h"
 #include "ns3/ipv4-global-routing-helper.h"
 //changes below
  #include "ns3/netanim-module.h"
  #include "ns3/wifi-module.h"
  #include "ns3/point-to-point-module.h"
  //*********************************
  #define LOGNAME_PREFIX "TCP_NetgearSetup_09March2021"
  NS_LOG_COMPONENT_DEFINE ("energyModelWiFiTCP");
  
 
 using namespace ns3;
 
 Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
 uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
 
//Configurable parameters************************
uint32_t payloadSize = 1460;                       /* Transport layer payload size in bytes. */
uint32_t dataPeriod = 1;                       /* Application data period in seconds. */
uint32_t delACKTimer_ms = 0;                     /* TCP delayed ACK timer in ms   */ 
uint32_t P2PLinkDelay_ms = 2.5;                  // Set this to be half of the expected RTT
double simulationTime = 20;                        /* Simulation time in seconds. */
//*******************************************************************

void
CalculateThroughput ()
{
  // This function has been modified to output the data received every second in Bytes
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double currentDataBytes = (sink->GetTotalRx () - lastTotalRx);
  //double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
  //std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  std::cout << now.GetSeconds () << "s: Data received in previous second in Bytes\t" << currentDataBytes << " Bytes" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (1000), &CalculateThroughput);
}
 // PHY state tracing - check log file
template <int node>
void PhyStateTrace (std::string context, Time start, Time duration, WifiPhyState state)
{
  std::stringstream ss;
  ss << "TI_ns3/stateLog_"<<LOGNAME_PREFIX <<"_node_" <<node << ".log";

  static std::fstream f (ss.str ().c_str (), std::ios::out);

  f << Simulator::Now ().GetSeconds () << "    state=" << state << " start=" << start << " duration=" << duration << std::endl;
}

//*************************************************************************
 int
 main (int argc, char *argv[])
 {
  uint32_t dataRatebps = payloadSize*8/dataPeriod;  
  std::string dataRate = std::to_string(dataRatebps) + std::string("bps");
  //std::string dataRate = "8000bps";                  /* Application layer datarate. */
  std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
  std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
  
  bool pcapTracing = true;                          /* PCAP Tracing is enabled or not. */
 
   /* Command line argument parser setup. */
   CommandLine cmd (__FILE__);
   cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
   cmd.AddValue ("dataRate", "Application data ate", dataRate);
   cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpNewReno, "
                 "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                 "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
   cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
   cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
   cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
   cmd.Parse (argc, argv);
 
   tcpVariant = std::string ("ns3::") + tcpVariant;
   // Select TCP variant
   if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
     {
       // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
       Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
       // the default protocol type in ns3::TcpWestwood is WESTWOOD
       Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
     }
   else
     {
       TypeId tcpTid;
       NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
       Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
     }
 
   /* Configure TCP Options */
   Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
 
   WifiMacHelper wifiMac;
   WifiHelper wifiHelper;
   wifiHelper.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);
 
   /* Set up Legacy Channel */
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (2.4e9));
 
   /* Setup Physical Layer */
   YansWifiPhyHelper wifiPhy;
   wifiPhy.SetChannel (wifiChannel.Create ());
   wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
   wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                       "DataMode", StringValue (phyRate),
                                       "ControlMode", StringValue ("HtMcs0"));
 
   NodeContainer networkNodes;
   networkNodes.Create (2);
   Ptr<Node> apWifiNode = networkNodes.Get (0);
   Ptr<Node> staWifiNode = networkNodes.Get (1);
 
   /* Configure AP */
   Ssid ssid = Ssid ("network");
   wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));
 
   NetDeviceContainer apDevice;
   apDevice = wifiHelper.Install (wifiPhy, wifiMac, apWifiNode);
 
   /* Configure STA */
   wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid));
 
   NetDeviceContainer staDevices;
   staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);
 
   /* Mobility model */
   MobilityHelper mobility;
   Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
   positionAlloc->Add (Vector (0.0, 0.0, 0.0));
   positionAlloc->Add (Vector (1.0, 1.0, 0.0));
 
   mobility.SetPositionAllocator (positionAlloc);
   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility.Install (apWifiNode);
   mobility.Install (staWifiNode);
 
   /* Internet stack */
   InternetStackHelper stack;
   stack.Install (networkNodes);
 
   Ipv4AddressHelper address;
   address.SetBase ("10.0.0.0", "255.255.255.0");
   Ipv4InterfaceContainer apInterface;
   apInterface = address.Assign (apDevice);
   Ipv4InterfaceContainer staInterface;
   staInterface = address.Assign (staDevices);
 
   /* Populate routing table */
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
   /* Install TCP Receiver on the access point */
   PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
   ApplicationContainer sinkApp = sinkHelper.Install (apWifiNode);
   sink = StaticCast<PacketSink> (sinkApp.Get (0));
 
   /* Install TCP/UDP Transmitter on the station */
   OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
   server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
   server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
   ApplicationContainer serverApp = server.Install (staWifiNode);
 
   /* Start Applications */
   sinkApp.Start (Seconds (0.0));
   serverApp.Start (Seconds (1.0));
   Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
 
   /* Enable Traces */
  if (pcapTracing)
    {
      std::stringstream ss1, ss2, ss3;
      wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
      ss1<<"TI_ns3/"<< LOGNAME_PREFIX <<"_AP";
      wifiPhy.EnablePcap (ss1.str(), apWiFiDevice);
      ss2<<"TI_ns3/"<< LOGNAME_PREFIX <<"_STA";
      wifiPhy.EnablePcap (ss2.str(), staWiFiDevice);
      ss3<<"TI_ns3/"<< LOGNAME_PREFIX <<"_P2P";
      pointToPoint.EnablePcapAll (ss3.str());
    }


  // For state tracing
  Config::Connect ("/NodeList/1/DeviceList/*/Phy/State/State", MakeCallback (&PhyStateTrace<1>));
 
   /* Start Simulation */
   Simulator::Stop (Seconds (simulationTime + 1));
   Simulator::Run ();
 
   double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6 * simulationTime));
 
   Simulator::Destroy ();
 
   if (averageThroughput < 50)
     {
       NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
       exit (1);
     }
   std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;
   return 0;
 }