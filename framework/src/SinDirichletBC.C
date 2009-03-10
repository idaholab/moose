#include "SinDirichletBC.h"
 
template<>
Parameters valid_params<SinDirichletBC>()
{
  Parameters params;
  params.set<Real>("initial")=0.0;
  params.set<Real>("final")=0.0;
  params.set<Real>("duration")=0.0;
  return params;
}
