#include "VacuumBC.h"

template<>
Parameters valid_params<VacuumBC>()
{
  Parameters params;
  params.set<Real>("alpha")=1;
  return params;
}
