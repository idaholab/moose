#include "BodyForce.h"
 
template<>
Parameters valid_params<BodyForce>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}
