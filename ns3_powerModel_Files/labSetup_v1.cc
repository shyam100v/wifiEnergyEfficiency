/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *    
 * This source file is modified from wifi-tcp.cc from the examples.
 * 
 * The example was to test TCP over 802.11n (with MPDU aggregation enabled).
 *
 * Network topology:
 *
 *   Ap    STA
 *   *      *
 *   |      |
 *   n1     n2
 *  **********
 *  Note:
 *  **********
 *  1. TCP Delayed ACK has been made as a configurable parameter. Set it to 0 ms for instant TCP ACKs.
 *  2. TCP server is now connected to the WiFi AP through a P2P node. The delay of this link should be half of the RTT. 
 *  
 *  To Do:
 *  1. Make the delay of the P2P link as a random variable - a normal variable. Keep the mean and variance as configurable parameters.
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
//-*********************************
#define LOGNAME_PREFIX "netgearSetup_10Mar2021"
NS_LOG_COMPONENT_DEFINE ("energyModelWiFiTCP");

using namespace ns3;

Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
//Configurable parameters************************
uint32_t payloadSize = 1460;                       /* Transport layer payload size in bytes. */
uint32_t dataPeriod = 1;                       /* Application data period in seconds. */
uint32_t delACKTimer_ms = 0;                     /* TCP delayed ACK timer in ms   */ 
uint32_t P2PLinkDelay_ms = 5;                  // Set this to be half of the expected RTT
double simulationTime = 20;                        /* Simulation time in seconds. */
//-*******************************************************************
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



//-**************************************************************************
// PHY state tracing - check log file
template <int node>
void PhyStateTrace (std::string context, Time start, Time duration, WifiPhyState state)
{
  std::stringstream ss;
  ss << "TI_ns3/stateLog_"<<LOGNAME_PREFIX <<"_node_" <<node << ".log";

  static std::fstream f (ss.str ().c_str (), std::ios::out);

  f << Simulator::Now ().GetSeconds () << "    state=" << state << " start=" << start << " duration=" << duration << std::endl;
}

//-*************************************************************************


int
main (int argc, char *argv[])
{
  uint32_t dataRatebps = payloadSize*8/dataPeriod;  
  std::string dataRate = std::to_string(dataRatebps) + std::string("bps");
  //std::string dataRate = "8000bps";                  /* Application layer datarate. */
  std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
  std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
  
  bool pcapTracing = true;                          /* PCAP Tracing is enabled or not. */

  LogComponentEnable ("energyModelWiFiTCP", LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_INFO));


  //tcpVariant = std::string ("ns3::") + tcpVariant;
  tcpVariant = std::string ("ns3::TcpNewReno");
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));


  /* Configure TCP Options */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
  Config::SetDefault ("ns3::TcpSocket::DelAckTimeout", TimeValue (MilliSeconds (delACKTimer_ms)));



  WifiMacHelper wifiMac;
  WifiHelper wifiHelper, wifiHelper_AP;
  //wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifiHelper.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);
  wifiHelper_AP.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (2.4e9));

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue (phyRate),
                                      "ControlMode", StringValue ("DsssRate1Mbps"),
                                      "NonUnicastMode", StringValue ("DsssRate1Mbps"));

  wifiHelper_AP.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
					"DataMode", StringValue ("DsssRate1Mbps"),
					"ControlMode", StringValue ("DsssRate1Mbps"),
                                      "NonUnicastMode", StringValue ("DsssRate1Mbps"));

  NodeContainer networkNodes;
  networkNodes.Create (3);
  Ptr<Node> apWifiNode = networkNodes.Get (0);
  Ptr<Node> staWifiNode = networkNodes.Get (1);
  Ptr<Node> TCPServerNode = networkNodes.Get (2);

  
  // Setup P2P nodes

  NodeContainer P2PNodes;
  P2PNodes.Add(apWifiNode);
  P2PNodes.Add(TCPServerNode);

  PointToPointHelper pointToPoint;
  std::stringstream delayString;
  delayString<<P2PLinkDelay_ms <<"ms";
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (delayString.str()));

  NetDeviceContainer P2Pdevices;
  P2Pdevices = pointToPoint.Install (P2PNodes);
  /* Configure AP */
  Ssid ssid = Ssid ("network");
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer apWiFiDevice;
  apWiFiDevice = wifiHelper_AP.Install (wifiPhy, wifiMac, apWifiNode);


  // Adding basic modes to AP - use this to change beacon data rates

 // Ptr<WifiRemoteStationManager> apStationManager = DynamicCast<WifiNetDevice>(apWiFiDevice.Get (0))->GetRemoteStationManager ();
  //apStationManager->AddBasicMode (WifiMode ("ErpOfdmRate6Mbps"));
  //DynamicCast<WifiNetDevice>(apWiFiDevice.Get (0))->GetRemoteStationManager ()->AddBasicMode (WifiMode ("ErpOfdmRate6Mbps"));

  /* Configure STA */
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer staWiFiDevice;
  staWiFiDevice = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);


// TO enable short GI for all nodes--------------------------
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HtConfiguration/ShortGuardIntervalSupported", BooleanValue (true));
//---------------------------------


  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 0.0, 0.0));
  positionAlloc->Add (Vector (-100.0, 0.0, 0.0));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apWifiNode);
  mobility.Install (staWifiNode);
  mobility.Install(TCPServerNode);


  
  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (networkNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apWiFiDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staWiFiDevice);
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer P2PInterfaces;
  P2PInterfaces = address.Assign (P2Pdevices);


  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Install TCP Receiver on the P2P TCP server */
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApp = sinkHelper.Install (TCPServerNode);
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (P2PInterfaces.GetAddress (1), 9)));
  server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  ApplicationContainer serverApp = server.Install (staWifiNode);



  /* Start Applications */
  sinkApp.Start (Seconds (0.0));
  serverApp.Start (Seconds (1.0));

  Simulator::Schedule (Seconds (1.0), &CalculateThroughput);

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

  //To enable ASCII traces
  AsciiTraceHelper ascii;
  std::stringstream ss;
  ss << "TI_ns3/ascii_"<<LOGNAME_PREFIX<<".tr" ;
  wifiPhy.EnableAsciiAll (ascii.CreateFileStream (ss.str()));
  //Changes below - Note: If position is changed here, it changes regardless of the code above.
  AnimationInterface anim("energyModelSim.xml");
  anim.SetConstantPosition(networkNodes.Get(0), 0.0, 0.0);
  anim.SetConstantPosition(networkNodes.Get(1), 10.0, 0.0);
  anim.SetConstantPosition(networkNodes.Get(2), -100.0, 0.0);
  anim.EnablePacketMetadata(true);
  //******************************************



  Simulator::Run ();

  double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6 * simulationTime));

  Simulator::Destroy ();
  std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;
  return 0;
}
