#include "ConvectiveFluxBC.h"

template<>
Parameters valid_params<ConvectiveFluxBC>()
{
  Parameters params;
  params.set<Real>("rate")=7500;
  params.set<Real>("value")=500;
  return params;
}

Real ConvectiveFluxBC::computeQpResidual()
{
  return -(_phi_face[_i][_qp]*_rate*(_value - _u_face[_qp]));
}
  
Real ConvectiveFluxBC::computeQpJacobian()
{
  return -(_phi_face[_i][_qp]*_rate*(_value - _phi_face[_j][_qp]));
}
