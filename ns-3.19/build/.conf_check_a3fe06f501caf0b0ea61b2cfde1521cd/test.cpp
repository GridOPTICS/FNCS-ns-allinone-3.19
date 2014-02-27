
#include<iostream>
#include<csimtime.h>
#include<integrator.h>

int main()
{
  TIME t = ::sim_comm::Integrator::getOneTimeStep();
  ::std::cout << t << ::std::endl;
  return 0;
}
