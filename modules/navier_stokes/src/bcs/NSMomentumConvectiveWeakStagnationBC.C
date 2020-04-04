//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMomentumConvectiveWeakStagnationBC.h"

registerMooseObject("NavierStokesApp", NSMomentumConvectiveWeakStagnationBC);

InputParameters
NSMomentumConvectiveWeakStagnationBC::validParams()
{
  InputParameters params = NSWeakStagnationBaseBC::validParams();
  params.addClassDescription("The convective part (sans pressure term) of the momentum equation "
                             "boundary integral evaluated at specified stagnation temperature, "
                             "stagnation pressure, and flow direction values.");
  params.addRequiredParam<unsigned>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  return params;
}

NSMomentumConvectiveWeakStagnationBC::NSMomentumConvectiveWeakStagnationBC(
    const InputParameters & parameters)
  : NSWeakStagnationBaseBC(parameters), _component(getParam<unsigned>("component"))
{
}

Real
NSMomentumConvectiveWeakStagnationBC::computeQpResidual()
{
  // Compute stagnation values
  Real T_s = 0.0, p_s = 0.0, rho_s = 0.0;
  staticValues(T_s, p_s, rho_s);

  // The specified flow direction, as a vector
  RealVectorValue s(_sx, _sy, _sz);

  // (rho_s * |u|^2 * s_k * (s.n)) * phi_i
  return (rho_s * this->velmag2() * s(_component) * this->sdotn()) * _test[_i][_qp];
}

Real
NSMomentumConvectiveWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.0;
}

Real
NSMomentumConvectiveWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.0;
}
