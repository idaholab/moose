//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Torque.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", Torque);
registerMooseObject("TensorMechanicsApp", ADTorque);

template <bool is_ad>
InputParameters
TorqueTempl<is_ad>::validParams()
{
  InputParameters params = TorqueParent<is_ad>::validParams();
  params.addClassDescription(
      "Apply a moment as tractions distributed over a surface around a "
      "pivot point. This should operate on the displaced mesh for large deformations.");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<Point>(
      "origin", "Reference point defining the location for the axis the torque is applied to");
  params.addRequiredParam<RealVectorValue>("direction", "Torque vector");
  params.addParam<FunctionName>(
      "factor", "1", "Prefactor for the force (can only be time dependent)");
  params.addRequiredParam<PostprocessorName>(
      "polar_moment_of_inertia", "Postprocessor that computes the polar moment of inertia");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  params.addParamNamesToGroup("alpha factor", "Advanced");
  params.addCoupledVar("displacements", "The displacements");
  return params;
}

template <bool is_ad>
TorqueTempl<is_ad>::TorqueTempl(const InputParameters & parameters)
  : TorqueParent<is_ad>(parameters),
    _component(libMesh::invalid_uint),
    _origin(this->template getParam<Point>("origin")),
    _torque(this->template getParam<RealVectorValue>("direction")),
    _factor(this->getFunction("factor")),
    _alpha(this->template getParam<Real>("alpha")),
    _pmi(this->getPostprocessorValue("polar_moment_of_inertia")),
    _ndisp(this->coupledComponents("displacements")),
    _dvars(_ndisp),
    _dummy_point()
{
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _dvars[i] = this->getVar("displacements", i)->number();
    if (this->_var.number() == _dvars[i])
      _component = i;
  }

  if (_component == libMesh::invalid_uint)
    this->paramError("variable",
                     "The kernel variable needs to be one of the 'displacements' variables");
  if (this->template getParam<bool>("use_displaced_mesh"))
    this->paramError("use_displaced_mesh", "This BC is only validated for small strains");
}

template <>
ADReal
TorqueTempl<true>::computeQpResidual()
{
  // local lever (distance to the origin)
  auto local_lever = this->_ad_q_points[_qp] - _origin;

  // force calculation
  auto local_force =
      _factor.value(_t + _alpha * _dt, _dummy_point) * _torque.cross(local_lever) / _pmi;

  return -local_force(_component) * _test[_i][_qp];
}

template <>
Real
TorqueTempl<false>::computeQpResidual()
{
  // local lever (distance to the origin)
  auto local_lever = this->_q_point[_qp] - _origin;

  // force calculation
  auto local_force =
      _factor.value(_t + _alpha * _dt, _dummy_point) * _torque.cross(local_lever) / _pmi;

  return -local_force(_component) * _test[_i][_qp];
}

template <>
Real
TorqueTempl<false>::componentJacobian(unsigned int component)
{
  // vector phi
  RealGradient phi;
  phi(component) = this->_phi[_j][_qp];

  // force calculation
  auto d_local_force = _factor.value(_t + _alpha * _dt, _dummy_point) * _torque.cross(phi) / _pmi;
  return -d_local_force(_component) * _test[_i][_qp];
}

template <>
Real
TorqueTempl<true>::componentJacobian(unsigned int)
{
  mooseError("This should never get called");
}

template <bool is_ad>
Real
TorqueTempl<is_ad>::computeQpJacobian()
{
  return componentJacobian(_component);
}

template <bool is_ad>
Real
TorqueTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _dvars[i])
      return componentJacobian(i);

  return 0.0;
}

template class TorqueTempl<false>;
template class TorqueTempl<true>;
