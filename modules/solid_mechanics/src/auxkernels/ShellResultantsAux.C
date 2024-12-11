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
      "Compute the local forces, bending moments and shear forces on shell elements");
  params.addParam<std::string>("base_name", "Mechanical property base name");
  params.addRequiredCoupledVar(
      "thickness",
      "Thickness of the shell. Can be supplied as either a number or a variable name.");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  MooseEnum output_resultant(
      "axial_force_1 axial_force_2 normal_force bending_moment_00 bending_moment_11 "
      "bending_moment_01 shear_force_01 shear_force_02 shear_force_12");
  params.addRequiredParam<MooseEnum>(
      "output_resultant",
      output_resultant,
      "The resultant type to output, calculated on the shell element.");

  return params;
}

ShellResultantsAux::ShellResultantsAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _thickness(coupledValue("thickness")),
    _resultant(getParam<MooseEnum>("output_resultant"))
{
  _t_qrule = std::make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = _t_qrule->get_points();
  _t_weights = _t_qrule->get_weights();
  _local_stress_t_points.resize(_t_points.size());
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    _local_stress_t_points[t] = &getMaterialProperty<RankTwoTensor>(
        _base_name + "local_stress_t_points_" + std::to_string(t));
  }
}

Real
ShellResultantsAux::computeValue()
{
  Real _shell_resultant = 0.0;

  switch (_resultant)
  {
    case 0: // Corresponds to "axial_force_1", normal force along the first local axis
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 0) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case 1: // Corresponds to "axial_force_2", normal force along the second local axis
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](1, 1) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case 2: // Corresponds to "normal force", normal force to the shell plane, it should be
            // always zero (plane stress condition)
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](2, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case 3: // Corresponds to "bending_moment_00", bending moment around first local axis
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](1, 1) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case 4: // Corresponds to "bending_moment_11", bending moment around second local axis
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](0, 0) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case 5: // Corresponds to "bending_moment_01", in-plane bending moment
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant -= (*_local_stress_t_points[i])[_qp](0, 1) * _t_points[i](0) *
                            _t_weights[i] * (_thickness[_qp] / 2) * (_thickness[_qp] / 2);
      }
      break;

    case 6: // Corresponds to "shear_force_01", in-plane shear force
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 1) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case 7: // Corresponds to "shear_force_02", out of plane shear force
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](0, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;

    case 8: // Corresponds to "shear_force_12", out of plane shear force
      for (unsigned int i = 0; i < _t_points.size(); ++i)
      {
        _shell_resultant +=
            (*_local_stress_t_points[i])[_qp](1, 2) * _t_weights[i] * (_thickness[_qp] / 2);
      }
      break;
      // Add other cases as needed

    default:
      mooseError("Unsupported output_resultant option: " + _resultant);
  }

  return _shell_resultant;
}
