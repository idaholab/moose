/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// This class is deprecated, use NSMomentumInviscidSpecifiedPressureBC instead.

// #include "NSPressureNeumannBC.h"
//
// template<>
// InputParameters validParams<NSPressureNeumannBC>()
// {
//   InputParameters params = validParams<NSIntegratedBC>();
//
//   // Required vars
//   params.addRequiredCoupledVar("pressure", "");
//
//   // Required parameters
//   params.addRequiredParam<unsigned>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
//   params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");
//
//   return params;
// }
//
//
//
// NSPressureNeumannBC::NSPressureNeumannBC(const std::string & name, InputParameters parameters)
//     : NSIntegratedBC(name, parameters),
//       // Coupled variables
//       _pressure(coupledValue("pressure")),
//
//       // Required parameters
//       _component(getParam<unsigned>("component")),
//       _gamma(getParam<Real>("gamma")),
//
//       // Pressure derivative computation object
//       _pressure_derivs(*this)
// {
// }
//
//
//
//
// Real NSPressureNeumannBC::computeQpResidual()
// {
//   return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
// }
//
//
//
//
//
// Real NSPressureNeumannBC::computeQpJacobian()
// {
//   return this->compute_jacobian(_component+1);  // <-- the on-diagonal variable number is _component+1
// }
//
//
//
// Real NSPressureNeumannBC::computeQpOffDiagJacobian(unsigned jvar)
// {
//   unsigned m = mapVarNumber(jvar);
//
//   return this->compute_jacobian(m);
// }
//
//
//
// Real NSPressureNeumannBC::compute_jacobian(unsigned m)
// {
//   return _normals[_qp](_component) * _pressure_derivs.get_grad(m) * _phi[_j][_qp] * _test[_i][_qp];
// }
