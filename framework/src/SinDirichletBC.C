#include "SinDirichletBC.h"
 
template<>
Parameters valid_params<SinDirichletBC>()
{
  Parameters params;
  params.set<Real>("tempzero")=0.0;
  params.set<Real>("tempmax")=0.0;
  params.set<Real>("timeduration")=0.0;
  return params;
}
