//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AveragePointSeparation.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"

#include "ComputeIncrementalBeamStrain.h"
#include "Assembly.h"
#include "NonlinearSystem.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", AveragePointSeparation);

defineLegacyParams(AveragePointSeparation);

InputParameters
AveragePointSeparation::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<std::vector<Point>>("first_point",
                                              "A list of first points in the numerical domain");
  params.addRequiredParam<std::vector<Point>>("last_point",
                                              "A list of last points in the numerical domain");
  params.addClassDescription(
      "Computes the average separation on deformed mesh between two sets of points");
  return params;
}

AveragePointSeparation::AveragePointSeparation(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _displacements(getParam<std::vector<VariableName>>("displacements")),

    _system(_subproblem.getSystem(_displacements[0])),
    _first_point(getParam<std::vector<Point>>("first_point")),
    _last_point(getParam<std::vector<Point>>("last_point")),
    _value(0)
{
  // fetch coupled variables and gradients (as stateful properties if necessary)
  _disp_num = std::vector<int>(_displacements.size());
  for (unsigned int i = 0; i < _displacements.size(); ++i)
    _disp_num[i] = _subproblem
                       .getVariable(_tid,
                                    _displacements[i],
                                    Moose::VarKindType::VAR_ANY,
                                    Moose::VarFieldType::VAR_FIELD_STANDARD)
                       .number();

  if(_first_point.size() != _last_point.size())
    mooseerror("first point and last point array should have the same size.");
}

void
AveragePointSeparation::execute()
{
  _value = 0.;
  Real sq_diff;
  for (unsigned int i = 0; i < _first_point.size(); ++i)
  {
    sq_diff = 0;
    for (unsigned int j = 0; j < _displacements.size(); ++j)
      sq_diff += Utility::pow<2>((_first_point[i](j) + _system.point_value(_disp_num[j], _first_point[i], false)) -
                                 (_last_point[i](j) + _system.point_value(_disp_num[j], _last_point[i], false)));

    _value += std::sqrt(sq_diff);
  }
  _value = _value / _first_point.size();
}

Real
AveragePointSeparation::getValue()
{
  return _value;
}
