#ifndef fncsns3Application
#define fncsns3Application

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include "ns3/system-mutex.h"
#include <map>
#include <integrator.h>
#include <objectcomminterface.h>
#include <message.h>
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"

using namespace std;

namespace ns3 {

class Socket;
class Packet;

class FNCSApplication : public Application
{
public:
  static TypeId
  GetTypeId (void);

  FNCSApplication();
  FNCSApplication (string objname);

  virtual ~FNCSApplication ();
  
  void setNameResolution(map<string,pair<Ipv4Address,uint16_t> >  &resolver);
  void setMarketToNodeMap(map<string, string>  &marketMap);
  void setName(string given);
protected:
  virtual void DoDispose (void);

private:
  
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  void gotMessage();
  void registerInterface(string objname);
  
  void receiveFNCSMessages();
  void send(sim_comm::Message *message);
  void handleRead (Ptr<Socket> socket);
  string name;
  sim_comm::ObjectCommInterface *in;
  bool notified;
  map<string,pair<Ipv4Address,uint16_t> > nodes;
  map<string, string> marketToNodeMap;
 
 // map<string, Ptr<Socket> > nodeSockets;
  
  Ptr<Socket> udpsocket;
  
  
};

} // namespace ns3


#endif
