/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMomentumPressureWeakStagnationBC.h"

template<>
InputParameters validParams<NSMomentumPressureWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBC>();

  // Required parameters
  params.addRequiredParam<unsigned>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}



NSMomentumPressureWeakStagnationBC::NSMomentumPressureWeakStagnationBC(const std::string & name, InputParameters parameters)
    : NSWeakStagnationBC(name, parameters),

      // Required parameters
      _component(getParam<unsigned>("component"))
{
}




Real NSMomentumPressureWeakStagnationBC::computeQpResidual()
{
  // Compute stagnation values
  Real T_s = 0., p_s = 0., rho_s = 0.;
  this->static_values(T_s, p_s, rho_s);

  // (p_s * n_k) * phi_i
  return (p_s * _normals[_qp](_component)) * _test[_i][_qp];
}




Real NSMomentumPressureWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.;
}




Real NSMomentumPressureWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.;
}




