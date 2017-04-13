/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMomentumConvectiveWeakStagnationBC.h"

template <>
InputParameters
validParams<NSMomentumConvectiveWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBaseBC>();
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
