#include "fncs-application.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/fncs.h"
#include "callback.h"
#include "ns3/packet.h"

namespace ns3{
  
  NS_LOG_COMPONENT_DEFINE ("FNCSApplication");
  NS_OBJECT_ENSURE_REGISTERED (FNCSApplication);
  
TypeId FNCSApplication::GetTypeId(void )
{
  static TypeId tid = TypeId ("ns3::FNCSApplication")
    .SetParent<Application> ()
    .AddConstructor<FNCSApplication> ()
    .AddAttribute("MyName","The name of the foreign object this node corresponds to",
		  StringValue(string()),
		  MakeStringAccessor(&FNCSApplication::name),
		  MakeStringChecker())
  ;
  return tid;
}

FNCSApplication::FNCSApplication()
{
  name="";
  in=NULL;
   this->udpsocket = 0;
   notified=false;
}


void FNCSApplication::DoDispose(void )
{
  ns3::Application::DoDispose();
}

void FNCSApplication::registerInterface(string objname)
{
 Ptr<SimulatorImpl> impl=Simulator::GetImplementation();
    
    Ptr<FncsSimulator> sim=DynamicCast<FncsSimulator,SimulatorImpl>(impl);
    
    if(sim==NULL)
       NS_FATAL_ERROR("FNCS Application can only be used with FNCS simulator");
    
    this->name=objname;
    this->in= sim_comm::Integrator::getCommInterface(this->name);
    
    sim_comm::CallBack<void,sim_comm::empty,sim_comm::empty,sim_comm::empty,sim_comm::empty> *notify
	=CreateObjCallback<FNCSApplication *, void (FNCSApplication::*)(), void>(this,&FNCSApplication::gotMessage);
	
    this->in->setMessageNotifier(notify);
}



FNCSApplication::FNCSApplication(string objname)
{
   registerInterface(objname);
    this->udpsocket = 0;
    notified=false;
}

FNCSApplication::~FNCSApplication()
{
  delete this->in;
  this->udpsocket=0;
}

void FNCSApplication::gotMessage()
{
  if(notified)
    return;
  
  //uint32_t id=this->udpsocket->GetNode()->GetId();
  if(Simulator::Now().GetSeconds()==0){
    Time zero=Seconds(0.1);
    Simulator::Schedule(zero,&FNCSApplication::receiveFNCSMessages,this);
  }
  else{
    Time zero=Seconds(0);
    Simulator::Schedule(zero,&FNCSApplication::receiveFNCSMessages,this);
  }
  notified=true;
}

void FNCSApplication::setName(string given){
  if(this->name.empty()){
      this->name=given;
      registerInterface(given);
  }
  else{
    NS_FATAL_ERROR("Application is already registered with FNCS");
  }
}

void FNCSApplication::setNameResolution(map< string, pair< Ipv4Address, uint16_t > >& resolver)
{
  this->nodes = resolver;
}

void FNCSApplication::StartApplication(void )
{
  if(name.empty())
    NS_FATAL_ERROR("FNCS App cannot start without a name set");

 if (udpsocket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      udpsocket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 1000);
      udpsocket->Bind (local);
      
    }
    
    udpsocket->SetRecvCallback(MakeCallback (&FNCSApplication::handleRead, this));
  
   
}

void FNCSApplication::StopApplication(void )
{
  if (udpsocket != 0) 
    {
      udpsocket->Close ();
      udpsocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}


void FNCSApplication::handleRead(Ptr< Socket > socket)
{
  Ptr<Packet> p;
  Address from;
   while (p = socket->RecvFrom (from))
    {
      if (p->GetSize () > 0)
        {
	  uint8_t *data=new uint8_t[p->GetSize()];
	  p->CopyData(data,p->GetSize());
	  //InetSocketAddress ifrom=InetSocketAddress::ConvertFrom(from);
	  //NS_LOG_INFO(this->name << ": Received " << p->GetSize() << " bytes from " << ifrom.GetIpv4());
	  //Ptr<Packet> cp=Create<Packet>(&data[4],p->GetSize()-4);
	  uint32_t headerSize=0;
	  memcpy(&headerSize,data,sizeof(uint32_t));
	  //possible bug!
	  sim_comm::Message *msg=new Message(&data[sizeof(uint32_t)],headerSize,&data[sizeof(uint32_t)+headerSize]);
	  //here we need to arrange to part in bcast messages
	  if(msg->isBroadCast()){
	    sim_comm::Message *msg2=msg;
	    msg=new Message(msg2->getFrom(),this->name,msg2->getTime(),msg2->getData(),msg2->getSize(),msg2->getTag());
	    delete msg2;
	  }
	  this->in->send(msg);
	  delete[] data;
        }
    }
}

void FNCSApplication::send(sim_comm::Message* message)
{
  uint8_t *header;
  const uint8_t *data;
  uint32_t headerSize,dataSize;
  
  data=message->getData();
  dataSize=message->getSize();
  message->serializeHeader(header,headerSize);
  
  uint8_t* combined=new uint8_t[headerSize+dataSize+sizeof(uint32_t)];
  memcpy(combined,&headerSize,sizeof(uint32_t));
  memcpy(&combined[sizeof(uint32_t)],header,headerSize);
  memcpy(&combined[headerSize+sizeof(uint32_t)],data,dataSize);
  if(message->isBroadCast()){
    NS_LOG_INFO(this->name << ":Sending BCAST MEssage");
    map<string, pair<Ipv4Address,uint16_t> >::iterator it=nodes.begin();
    
    for(;it!=nodes.end();++it){
      Ptr<Packet> p=Create<Packet>(combined,headerSize+dataSize+sizeof(uint32_t));
      pair<Ipv4Address, uint16_t> pR=it->second;
      InetSocketAddress destInet(pR.first,pR.second);
      this->udpsocket->SendTo(p,0,destInet);
    }
  }
  else{
    map<string, pair<Ipv4Address,uint16_t> >::iterator it=nodes.find(message->getTo());
    
    if(it==nodes.end())
      NS_FATAL_ERROR(this->name << ":CANNOT FIND TO IN NAMES REGISTER!!! "<<message->getTo());
    
    NS_LOG_INFO(this->name << ":Sending t0 " << message->getTo());
  
    Ptr<Packet> p=Create<Packet>(combined,headerSize+dataSize+sizeof(uint32_t)); 
    pair<Ipv4Address, uint16_t> pR=it->second;
    InetSocketAddress destInet(pR.first,pR.second);
    this->udpsocket->SendTo(p,0,destInet);
  }
  
  delete[] combined;
  delete[] header;
}

void FNCSApplication::receiveFNCSMessages()
{

  if(this->in==NULL)
    NS_FATAL_ERROR("FNCS integration is not set proprely!");
  
  //NS_LOG_INFO("FNCS has " << this->in->getInboxMessagesCount());
  
  while(this->in->hasMoreMessages()){
    sim_comm::Message *msg=this->in->getNextInboxMessage();
    if(msg!=nullptr){
      send(msg);
      delete msg;
    }
  }
  
  notified=false;
}



}


