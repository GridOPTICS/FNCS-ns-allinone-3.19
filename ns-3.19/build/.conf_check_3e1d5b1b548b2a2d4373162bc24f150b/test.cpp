
#include<csimtime.h>
#include<integrator.h>

int main()
{
  TIME t = sim_comm::Integrator::getOneTimeStep();
  return 0;
}
