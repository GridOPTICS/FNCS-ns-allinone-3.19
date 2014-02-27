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


#ifndef FNCSHELPER_H
#define FNCSHELPER_H

#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-address.h"
#include "ns3/simulator.h"
#include <ns3/nstime.h>
#include <ns3/application-container.h>
#include <ns3/node-container.h>
#include "ns3/system-mutex.h"
#include "integrator.h"

using namespace std;

namespace ns3{
  
class FNCSApplicationHelper
{

private:
   
    ObjectFactory fncsappFactory;
    
public:
    FNCSApplicationHelper();
    virtual ~FNCSApplicationHelper();

    ApplicationContainer SetApps(vector<string> names,NodeContainer nodes);
    ApplicationContainer SetApps(vector<string> names);
    
   
};

}


#endif //FNCSHELPER_H
