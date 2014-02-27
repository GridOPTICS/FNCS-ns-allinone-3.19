/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "fncs.h"
#include "ns3/log.h"
#include "zmqnetworkinterface.h"
#include "speculationtimecalculationstrategy.h"

NS_LOG_COMPONENT_DEFINE ("FNCS_SIMULATOR");



namespace ns3 {
  
NS_OBJECT_ENSURE_REGISTERED (FncsSimulator);

TypeId
FncsSimulator::GetTypeId (void)
{
  //std::cout << "cargildim" << std::endl;
  static TypeId tid = TypeId ("ns3::FncsSimulator")
    .SetParent<Object> ()
    .AddConstructor<FncsSimulator> ()
  ;
  return tid;
}

FncsSimulator::FncsSimulator()
{
  this->currentTime=0;
  this->currentContext = 0xffffffff;
  this->currentScheduler=NULL;
  this->stopped=true;
  Integrator::initIntegrator("configns3.json",0);
  //MpiNetworkInterface *interface=new MpiNetworkInterface(MPI_COMM_WORLD,true);
  //ZmqNetworkInterface *interface=new ZmqNetworkInterface(true);
  //Integrator::initIntegratorCommunicationSim(interface,NANOSECONDS,51000000000,0);
  //IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(NANOSECONDS,300000000000);
  //InfinitySpeculationTimeStrategy *st=new InfinitySpeculationTimeStrategy(NANOSECONDS);
  //Integrator::initIntegratorOptimisticCommLowOverhead(interface,NANOSECONDS,51000000000,0,300000000000,st);
  //Integrator::initIntegratorConservativeSleepingComm(interface,NANOSECONDS,2000000000,0);
  //sim_comm::CallBack<uint64_t,sim_comm::empty,sim_comm::empty,sim_comm::empty> *timerCallback=sim_comm::CreateCallback(getCurrentTime);
  sim_comm::CallBack<uint64_t,sim_comm::empty,sim_comm::empty,sim_comm::empty,sim_comm::empty> *timerCallback=sim_comm::CreateObjCallback<FncsSimulator *, uint64_t (FncsSimulator::*)(), uint64_t>(this, &FncsSimulator::currentTimeInternal);
  
  Integrator::setTimeCallBack(timerCallback);
  
}

void FncsSimulator::FinalizeRegistrations()
{
  Integrator::finalizeRegistrations();
}


FncsSimulator::~FncsSimulator()
{
  
}

void FncsSimulator::SetScheduler(ObjectFactory schedulerFactory)
{
   Ptr<Scheduler> scheduler = schedulerFactory.Create<Scheduler> ();

  if (this->currentScheduler!=NULL)
    {
      while (!this->currentScheduler->IsEmpty ())
        {
          Scheduler::Event next = this->currentScheduler->RemoveNext ();
          scheduler->Insert (next);
        }
    }
  this->currentScheduler = scheduler;
}

void FncsSimulator::DoDispose(void )
{
    if(this->currentScheduler!=NULL){
      while(!this->currentScheduler->IsEmpty()){
	Scheduler::Event next = this->currentScheduler->RemoveNext ();
	next.impl->Unref ();
      }
    }
    this->currentScheduler = NULL;
    ns3::SimulatorImpl::DoDispose();
}

void FncsSimulator::Destroy()
{
   while (!this->scheduleAtDestroyTime.empty ())
    {
      Ptr<EventImpl> ev = this->scheduleAtDestroyTime.front ().PeekEventImpl ();
      this->scheduleAtDestroyTime.pop_front ();
      NS_LOG_LOGIC ("handle destroy " << ev);
      if (!ev->IsCancelled ())
        {
          ev->Invoke ();
        }
    }
    Integrator::stopIntegrator();
  
}

uint64_t FncsSimulator::currentTimeInternal()
{
  return this->currentTime;
}


Time FncsSimulator::Now(void ) const
{
  return TimeStep(this->currentTime);
}


Time FncsSimulator::Next(void ) const
{
  NS_ASSERT (!currentScheduler->IsEmpty ());
  Scheduler::Event ev = currentScheduler->PeekNext ();
  return TimeStep(ev.key.m_ts);
}

void FncsSimulator::invokeNextEvent(void )
{
  Scheduler::Event next;
  {
    CriticalSection cs(this->m_mutex); 
    next= this->currentScheduler->RemoveNext ();
    NS_ASSERT_MSG(next.key.m_ts >= this->currentTime, "Failed in time event time " << next.key.m_ts << " sim time " << this->currentTime);
    this->currentTime = next.key.m_ts;
  
  }

  NS_ASSERT (next.key.m_ts < this->grantedTime);
  //NS_LOG_INFO ("handle " << next.key.m_ts);
  this->currentContext = next.key.m_context;
  this->currentUid = next.key.m_uid;
  next.impl->Invoke();
  next.impl->Unref ();
}


uint32_t FncsSimulator::GetSystemId() const
{	//we don't know rank in fncs simulatr!
  return 0;
}

void FncsSimulator::Run()
{
  this->stopped= false;
  this->FinalizeRegistrations();
  
  while (!this->stopped) 
    {
      uint64_t m_ts;
      bool empty;
      //Integrator::grantTimeCompleted(this->currentTime);
      {
	CriticalSection cs(this->m_mutex);
        empty = this->currentScheduler->IsEmpty();
	if(!empty){
        	m_ts=this->currentScheduler->PeekNext().key.m_ts;
		if(this->grantedTime < this->currentScheduler->PeekNext().key.m_ts)
		  this->currentTime=grantedTime;
		Integrator::timeStepStart(m_ts);
	}
	else{
	  this->currentTime=this->grantedTime;
	  Integrator::timeStepStart(this->currentTime);
	}
      }
      if(this->grantedTime > this->currentScheduler->PeekNext().key.m_ts){ //if this condition does not hold we need resync
		this->invokeNextEvent();	
      }
      else{ //we are guarented that until this time we won't get a new message
	  this->currentTime=this->grantedTime;
      }
		//uint64_t oldgrTime=this->grantedTime;
      this->grantedTime=Integrator::getNextTime(this->currentTime,m_ts);
      //this->currentTime=this->grantedTime;
      if(Integrator::isFinished()){
	this->stopped=true;
	break; //break is needed here, we don't want simulator to dispatch the next event, as it can place the integrator in an incosistant state.
      }
    }

}

void FncsSimulator::Cancel(const EventId& ev)
{
   if (!IsExpired (ev))
    {
      ev.PeekEventImpl ()->Cancel ();
    }
}

void FncsSimulator::Stop(void )
{
  Integrator::stopIntegrator();
  this->stopped=true;
}

void FncsSimulator::Stop(const Time& time)
{
  ns3::Simulator::Schedule(time, &Simulator::Stop);
}

bool FncsSimulator::IsExpired(const ns3::EventId& ev) const
{

  if (ev.GetUid () == 2)
    {
      if (ev.PeekEventImpl () == 0
          || ev.PeekEventImpl ()->IsCancelled ())
        {
          return true;
        }
      // destroy events.
      for (list<EventId>::const_iterator i = this->scheduleAtDestroyTime.begin (); i != this->scheduleAtDestroyTime.end (); i++)
        {
          if (*i == ev)
            {
              return false;
            }
        }
      return true;
    }
  if (ev.PeekEventImpl () == 0
      || ev.GetTs () < this->currentTime
      || (ev.GetTs () == this->currentTime
          && ev.GetUid () <= this->currentUid)
      || ev.PeekEventImpl ()->IsCancelled ())
    {
      return true;
    }
  else
    {
      return false;
    }
}


uint32_t FncsSimulator::GetContext(void ) const
{
  return this->currentContext;
}

Time FncsSimulator::GetMaximumSimulationTime(void ) const
{
  return TimeStep (0x7fffffffffffffffLL);
}


void FncsSimulator::RunOneEvent(void )
{
  NS_LOG(LOG_ERROR,"This operation is not supported with FNCS simulator");
}

bool FncsSimulator::IsFinished(void ) const
{
  return this->currentScheduler->IsEmpty () || this->stopped;
}

void FncsSimulator::Remove(const ns3::EventId& ev)
{
  if (ev.GetUid () == 2)
    {
      // destroy events.
      for (list<EventId>::iterator i = this->scheduleAtDestroyTime.begin (); i !=this->scheduleAtDestroyTime.end (); i++)
        {
          if (*i == ev)
            {
              this->scheduleAtDestroyTime.erase (i);
              break;
            }
        }
      return;
    }
  if (IsExpired (ev))
    {
      return;
    }
  Scheduler::Event event;
  event.impl = ev.PeekEventImpl ();
  event.key.m_ts = ev.GetTs ();
  event.key.m_context = ev.GetContext ();
  event.key.m_uid = ev.GetUid ();
  {
    CriticalSection cs(this->m_mutex);
    this->currentScheduler->Remove (event);
  }
  event.impl->Cancel ();
  event.impl->Unref ();
}

Time FncsSimulator::GetDelayLeft(const ns3::EventId& id) const
{
  if (IsExpired (id))
    {
      return TimeStep (0);
    }
  else
    {
      return TimeStep (id.GetTs () - this->currentTime);
    }
}

EventId FncsSimulator::ScheduleDestroy(EventImpl* event)
{
  EventId id (Ptr<EventImpl> (event, false), this->currentTime, 0xffffffff, 2);
  this->scheduleAtDestroyTime.push_back (id);
  return id;
}

EventId FncsSimulator::Schedule(const ns3::Time& time, EventImpl* event)
{
  
  Scheduler::Event ev;
  ev.impl = event;
  ev.key.m_context = GetContext ();
  ev.key.m_uid = this->ids++;
 
    Time tAbsolute = time + TimeStep (this->currentTime);

    NS_ASSERT (tAbsolute.IsPositive ());
    NS_ASSERT (tAbsolute >= TimeStep (this->currentTime));
    
    ev.key.m_ts = static_cast<uint64_t> (tAbsolute.GetTimeStep ());
    
    //this->ids++;
    this->currentScheduler->Insert (ev);
    //NS_LOG_INFO("SCHEDULED "<<ev.key.m_ts);
 
  return EventId (event, ev.key.m_ts, ev.key.m_context, ev.key.m_uid);
}

/**
 * Not theard safe use from same thread!
 */
EventId FncsSimulator::ScheduleNow(EventImpl* event)
{

  Scheduler::Event ev;
  ev.impl = event;
  ev.key.m_context = GetContext ();
  ev.key.m_uid=this->ids++;
    ev.key.m_ts = this->currentTime;
    currentScheduler->Insert (ev);
    //NS_LOG_INFO("Scheduled Now "<<ev.key.m_ts);
 
  return EventId (event, ev.key.m_ts, ev.key.m_context, ev.key.m_uid);
}

void FncsSimulator::ScheduleWithContext(uint32_t context, const ns3::Time& time, EventImpl* event)
{
  NS_LOG_FUNCTION (this << context << time.GetTimeStep () << this->currentTime << event);
  {
    CriticalSection cs(this->m_mutex);
    Scheduler::Event ev;
    ev.impl = event;
    ev.key.m_ts =this->currentTime + time.GetTimeStep ();
    ev.key.m_context = context;
    ev.key.m_uid = this->ids;
    this->ids++;
    this->currentScheduler->Insert (ev);
    //NS_LOG_INFO("Scheduled with context "<< ev.key.m_ts); 
 }
  

}

}

