/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMassArtificialCompressibility.h"

template<>
InputParameters validParams<INSMassArtificialCompressibility>()
{
  InputParameters params = validParams<Kernel>();

  params.addParam<Real>("penalty", 1e-4, "Penalty value is used to relax the incompressibility");

  return params;
}


INSMassArtificialCompressibility::INSMassArtificialCompressibility(const InputParameters & parameters) :
  Kernel(parameters),
  // penalty value
  _penalty(getParam<Real>("penalty"))

{
}

Real INSMassArtificialCompressibility::computeQpResidual()
{
  // penalty*p*q
  return _penalty*_u[_qp]* _test[_i][_qp];
}

Real INSMassArtificialCompressibility::computeQpOffDiagJacobian(unsigned /* jvar */)
{
  // does not couple any variables
  return 0;
}

Real INSMassArtificialCompressibility::computeQpJacobian()
{
  // Derivative wrt to p
  return _penalty*_phi[_j][_qp]* _test[_i][_qp];
}
