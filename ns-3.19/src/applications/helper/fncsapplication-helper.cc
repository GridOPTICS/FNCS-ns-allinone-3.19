/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "fncsapplication-helper.h"
#include "ns3/fncs-application.h"
#include "ns3/fncs.h"
#include <ns3/ipv4.h>
#include "ns3/log.h"
#include <ns3/boolean.h>
#include "ns3/node-list.h"


namespace ns3{

NS_LOG_COMPONENT_DEFINE ("FNCS-Application-Helper");

FNCSApplicationHelper::FNCSApplicationHelper()
{

  Ptr<SimulatorImpl> impl=Simulator::GetImplementation();
  
  Ptr<FncsSimulator> temp=DynamicCast<FncsSimulator,SimulatorImpl>(impl);
  
  if(temp==NULL)
    NS_FATAL_ERROR("FNCS Application can only be used with FNCS simulator");
}


FNCSApplicationHelper::~FNCSApplicationHelper(){
}



ApplicationContainer FNCSApplicationHelper::SetApps(vector< string > names, NodeContainer nodes)
{
  if(names.size()<nodes.GetN())
     NS_FATAL_ERROR("Not enough names to set fncs apps!");
  map< string, pair<Ipv4Address,uint16_t> > nameMap; 
  uint16_t port=1000;
  ApplicationContainer toReturn;
  vector<Ptr<FNCSApplication> > memkill;
  //Node 0 is the initiator.
  for(uint32_t i=0;i<nodes.GetN();i++){
     if (nodes.Get(i)->GetNDevices()<2)
	  NS_FATAL_ERROR("Nodes should be assigned IP addresses before calling SetGLDApps!!!");
     Ptr<FNCSApplication> app=new FNCSApplication(names[i]);
     Ptr<Ipv4> net=nodes.Get(i)->GetObject<Ipv4>();
     Ipv4InterfaceAddress add=net->GetAddress(1,0);
     Ipv4Address nodeadd=add.GetLocal();
     nameMap.insert(pair<string, pair<Ipv4Address,uint16_t> >(names[i],pair<Ipv4Address,uint16_t>(nodeadd,port)));
     memkill.push_back(app);
  }
  
  for(uint32_t i=0;i<nodes.GetN();i++){
    memkill[i]->setNameResolution(nameMap);
    toReturn.Add(memkill[i]);
    nodes.Get(i)->AddApplication(memkill[i]);
  }
  
  
  return toReturn;

}


ApplicationContainer FNCSApplicationHelper::SetApps(vector< string > names)
{
  //map< string, pair<Ipv4Address,uint16_t> > nameMap; 
  //uint16_t port=1000;
  ApplicationContainer toReturn;
  //vector<Ptr<FNCSApplication> > memkill;
  
  /*for(uint32_t i=0;i<NodeList.GetNNodes();i++){
     if (NodeList.GetNode(i)->GetNDevices()<2)
	  NS_FATAL_ERROR("Nodes should be assigned IP addresses before calling SetGLDApps!!!");
     Ptr<FNCSApplication> app=new FNCSApplication(names[i]);
     Ptr<Ipv4> net=NodeList.GetNode(i)->GetObject<Ipv4>();
     Ipv4InterfaceAddress add=net->GetAddress(1,0);
     Ipv4Address nodeadd=add.GetLocal();
     nameMap.insert(pair<string, pair<Ipv4Address,uint16_t> >(names[i],pair<Ipv4Address,uint16_t>(nodeadd,port)));
     memkill.push_back(app);
  }
  
  for(uint32_t i=0;i<NodeList.GetNNodes();i++){
    memkill[i]->setNameResolution(nameMap);
    toReturn.Add(memkill[i]);
    NodeList.GetNode(i)->AddApplication(memkill[i]);
  }*/
  
  
  return toReturn;
}



}
