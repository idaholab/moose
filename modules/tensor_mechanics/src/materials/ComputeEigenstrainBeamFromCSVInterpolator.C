//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainBeamFromCSVInterpolator.h"
#include "CSVInterpolator.h"

template <>
InputParameters
validParams<ComputeEigenstrainBeamFromCSVInterpolator>()
{
  InputParameters params = validParams<ComputeEigenstrainBeamBase>();
  params.addClassDescription("Computes the displacement and rotational eigenstrains from a two "
                             "different sets of csv files.");
  params.addParam<UserObjectName>("disp_eigenstrain_uo",
                                  "Name of userobject that reads the "
                                  "csv files for "
                                  "displacement eigenstrains.");
  params.addParam<UserObjectName>("rot_eigenstrain_uo",
                                  "Name of userobject that reads the "
                                  "csv files for "
                                  "rotational eigenstrains.");
  params.addRequiredParam<unsigned int>(
      "to_component", "Component to which the information from csv files needs to be mapped to.");
  params.addRequiredParam<std::vector<Real>>(
      "position_vector",
      "Vector containing 2 entries specifing the position in directions other "
      "than to_component. For example, if to_component is set to 1, then the "
      "first entry would correspond to the x position and the second entry "
      "would correspond to the z position.");
  return params;
}

ComputeEigenstrainBeamFromCSVInterpolator::ComputeEigenstrainBeamFromCSVInterpolator(
    const InputParameters & parameters)
  : ComputeEigenstrainBeamBase(parameters),
    _to_component(getParam<unsigned int>("to_component")),
    _position_vector(getParam<std::vector<Real>>("position_vector")),
    _other_components(2),
    _disp_eigenstrain_uo(isParamValid("disp_eigenstrain_uo")
                             ? &getUserObject<CSVInterpolator>("disp_eigenstrain_uo")
                             : nullptr),
    _rot_eigenstrain_uo(isParamValid("rot_eigenstrain_uo")
                            ? &getUserObject<CSVInterpolator>("rot_eigenstrain_uo")
                            : nullptr),
    _ndisp(isParamValid("disp_eigenstrain_uo") ? _disp_eigenstrain_uo->getNumberOfVariables() : 0),
    _nrot(isParamValid("rot_eigenstrain_uo") ? _rot_eigenstrain_uo->getNumberOfVariables() : 0)
{
  if (_to_component > 2)
    mooseError(
        "ComputeEigenstrainBeamFromCSVInterpolator: to_component should be between 0 and 2.");

  if (_position_vector.size() != 2)
    mooseError(
        "ComputeEigenstrainBeamFromCSVInterpolator: position_vector should contain 2 entries.");

  if (!isParamValid("disp_eigenstrain_uo") && !isParamValid("rot_eigenstrain_uo"))
    mooseError("ComputeEigenstrainBeamFromCSVInterpolator: Neither disp_eigenstrain_uo nor "
               "rot_eigenstrain_uo "
               "have been provided as input.");

  if (_to_component == 0)
  {
    _other_components[0] = 1;
    _other_components[1] = 2;
  }
  else if (_to_component == 1)
  {
    _other_components[0] = 0;
    _other_components[1] = 2;
  }
  else if (_to_component == 2)
  {
    _other_components[0] = 0;
    _other_components[1] = 1;
  }
}

void
ComputeEigenstrainBeamFromCSVInterpolator::computeQpEigenstrain()
{
  for (unsigned int i = 0; i < 3; ++i)
  {
    _disp_eigenstrain[_qp](i) = 0.0;
    _rot_eigenstrain[_qp](i) = 0.0;
  }

  // Check if q point has the same position as in the position vector in the directions other than
  // to_component.
  if (MooseUtils::absoluteFuzzyEqual(_q_point[_qp](_other_components[0]), _position_vector[0]) &&
      MooseUtils::absoluteFuzzyEqual(_q_point[_qp](_other_components[1]), _position_vector[1]))
  {
    std::vector<Real> disp_value(_ndisp, 0.0);
    std::vector<Real> rot_value(_nrot, 0.0);
    if (_ndisp != 0)
    {
      disp_value = _disp_eigenstrain_uo->getValue(_q_point[_qp](_to_component), _t);
      for (unsigned int i = 0; i < _ndisp; ++i)
        _disp_eigenstrain[_qp](i) = disp_value[i];
    }

    if (_nrot != 0)
    {
      rot_value = _rot_eigenstrain_uo->getValue(_q_point[_qp](_to_component), _t);
      for (unsigned int i = 0; i < _nrot; ++i)
        _rot_eigenstrain[_qp](i) = rot_value[i];
    }
  }
}
