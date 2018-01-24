/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMomentumPressureWeakStagnationBC.h"

template <>
InputParameters
validParams<NSMomentumPressureWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBaseBC>();
  params.addClassDescription("This class implements the pressure term of the momentum equation "
                             "boundary integral for use in weak stagnation boundary conditions.");
  params.addRequiredParam<unsigned int>(
      "component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  return params;
}

NSMomentumPressureWeakStagnationBC::NSMomentumPressureWeakStagnationBC(
    const InputParameters & parameters)
  : NSWeakStagnationBaseBC(parameters), _component(getParam<unsigned int>("component"))
{
}

Real
NSMomentumPressureWeakStagnationBC::computeQpResidual()
{
  // Compute stagnation values
  Real T_s = 0.0, p_s = 0.0, rho_s = 0.0;
  staticValues(T_s, p_s, rho_s);

  // (p_s * n_k) * phi_i
  return (p_s * _normals[_qp](_component)) * _test[_i][_qp];
}

Real
NSMomentumPressureWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.0;
}

Real
NSMomentumPressureWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.0;
}
