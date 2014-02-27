/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FNCS_SIM
#define FNCS_SIM

#include "ns3/simulator-impl.h"
#include "ns3/simulator.h"
#include "ns3/scheduler.h"
#include "ns3/event-impl.h"
#include "ns3/ptr.h"
#include "ns3/system-mutex.h"

#include <list>

#include "integrator.h"

using namespace sim_comm;

namespace ns3 {


class FncsSimulator : public SimulatorImpl{
private:
  Ptr<Scheduler> currentScheduler;
  list<EventId> scheduleAtDestroyTime;
  virtual void DoDispose(void );
  void invokeNextEvent(void);
  uint64_t currentTime;
  uint32_t currentContext;
  uint64_t grantedTime;
  uint32_t currentUid;
  uint32_t ids;
  mutable SystemMutex m_mutex;

  bool stopped;
  uint64_t currentTimeInternal();
public:
    
    static TypeId GetTypeId (void);
    FncsSimulator();
    virtual ~FncsSimulator();
    virtual void Destroy();
    virtual void Cancel(const ns3::EventId& ev);
    virtual Time GetDelayLeft(const ns3::EventId& id) const;
    virtual uint32_t GetContext(void ) const;
    virtual Time GetMaximumSimulationTime(void ) const;
    virtual uint32_t GetSystemId() const;
    virtual bool IsExpired(const ns3::EventId& ev) const;
    virtual bool IsFinished(void ) const;
    virtual Time Next(void ) const;
    virtual Time Now(void ) const;
    virtual void Remove(const ns3::EventId& ev);
    virtual void Run(void );
    virtual void RunOneEvent(void );
    virtual EventId Schedule(const ns3::Time& time, EventImpl* event);
    virtual EventId ScheduleDestroy(EventImpl* event);
    virtual EventId ScheduleNow(EventImpl* event);
    virtual void ScheduleWithContext(uint32_t context, const ns3::Time& time, EventImpl* event);
    virtual void SetScheduler(ObjectFactory schedulerFactory);
    virtual void Stop(void );
    virtual void Stop(const ns3::Time& time);
    void FinalizeRegistrations();
};

}

#endif /* __FNCS-SIMULATOR_H__ */

