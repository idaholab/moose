//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMGcIntegral.h"

registerMooseObject("ShiftedBoundaryMethodApp", CZMGcIntegral);
registerMooseObject("ShiftedBoundaryMethodApp", ADCZMGcIntegral);

template <bool is_ad>
InputParameters
CZMGcIntegralTempl<is_ad>::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addParam<std::string>("base_name", "Material property base name");
  params.addClassDescription(
      "Integrate the interface traction over the displacement jump to obtain the cohesive work of "
      "separation Gc.");
  return params;
}

template <bool is_ad>
CZMGcIntegralTempl<is_ad>::CZMGcIntegralTempl(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _interface_traction_new(getGenericMaterialPropertyByName<RealVectorValue, is_ad>(
        _base_name + "interface_traction")),
    _interface_traction_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_traction")),
    _interface_displacement_jump_new(getGenericMaterialPropertyByName<RealVectorValue, is_ad>(
        _base_name + "interface_displacement_jump")),
    _interface_displacement_jump_old(
        getMaterialPropertyOldByName<RealVectorValue>(_base_name + "interface_displacement_jump")),
    _gc_integral(declarePropertyByName<Real>(_base_name + "gc_integral")),
    _gc_integral_old(getMaterialPropertyOldByName<Real>(_base_name + "gc_integral"))
{
}

template <bool is_ad>
void
CZMGcIntegralTempl<is_ad>::initQpStatefulProperties()
{
  _gc_integral[_qp] = 0;
}

template <bool is_ad>
void
CZMGcIntegralTempl<is_ad>::computeQpProperties()
{
  // Trapezoidal increment of the cohesive work: 0.5 * (T_new + T_old) . (delta_new - delta_old).
  const auto gc_integral_inc =
      0.5 * (_interface_traction_new[_qp] + _interface_traction_old[_qp]) *
      (_interface_displacement_jump_new[_qp] - _interface_displacement_jump_old[_qp]);

  _gc_integral[_qp] = _gc_integral_old[_qp] + MetaPhysicL::raw_value(gc_integral_inc);
}

template class CZMGcIntegralTempl<false>;
template class CZMGcIntegralTempl<true>;
