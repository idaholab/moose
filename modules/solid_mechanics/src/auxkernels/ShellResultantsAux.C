//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShellResultantsAux.h"
#include "ArbitraryQuadrature.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerMooseObject("SolidMechanicsApp", ShellResultantsAux);

InputParameters
ShellResultantsAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes the local forces, bending moments and shear forces acting on shell elements");
  params.addParam<std::string>("base_name", "Mechanical property base name");
  params.addRequiredCoupledVar(
      "thickness",
      "Thickness of the shell. Can be supplied as either a number or a variable name.");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  MooseEnum stress_resultant(
      "axial_force_0 axial_force_1 normal_force bending_moment_0 bending_moment_1 "
      "bending_moment_01 shear_force_01 shear_force_02 shear_force_12");
  params.addRequiredParam<MooseEnum>(
      "stress_resultant",
      stress_resultant,
      "The stress resultant type to output, calculated on the shell element.");

  return params;
}

ShellResultantsAux::ShellResultantsAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _thickness(coupledValue("thickness")),
    _resultant(getParam<MooseEnum>("stress_resultant").getEnum<ResultantType>())
{
  _t_qrule = std::make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = _t_qrule->get_points();
  _t_weights = _t_qrule->get_weights();
  _local_stress_t_points.resize(_t_points.size());

  for (const auto t : index_range(_t_points))
    _local_stress_t_points[t] = &getMaterialProperty<RankTwoTensor>(
        _base_name + "local_stress_t_points_" + std::to_string(t));
}

Real
ShellResultantsAux::computeValue()
{
  Real _shell_resultant = 0.0;

  switch (_resultant)
  {
    case ResultantType::axial_force_0:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 0) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::axial_force_1:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](1, 1) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::normal_force:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](2, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::bending_moment_0:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](1, 1) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::bending_moment_1:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](0, 0) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::bending_moment_01:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](0, 1) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::shear_force_01:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 1) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::shear_force_02:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case ResultantType::shear_force_12:
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](1, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;
  }

  return _shell_resultant;
}
