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


registerMooseObject("MooseApp", AveragePointSeparation);

defineLegacyParams(AveragePointSeparation);

InputParameters
AveragePointSeparation::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>(
      "disp_x", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<VariableName>(
      "disp_y", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<VariableName>(
      "disp_z", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<std::vector<Point>>("first_point",
                                              "A list of first points in the numerical domain");
  params.addRequiredParam<std::vector<Point>>("last_point",
                                              "A list of last points in the numerical domain");
  params.addClassDescription("Compute the average separation between a list of two specified locations");
  return params;
}

AveragePointSeparation::AveragePointSeparation(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var_0(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("disp_x"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _var_1(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("disp_y"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _var_2(_subproblem
                    .getVariable(_tid,
                                 parameters.get<VariableName>("disp_z"),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
                    .number()),
    _disp_num(std::vector<int> {0, 1, 2}),
    _system(_subproblem.getSystem(getParam<VariableName>("disp_x"))),
    _first_point(getParam<std::vector<Point>>("first_point")),
    _last_point(getParam<std::vector<Point>>("last_point")),
    _value(0)
{
  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  // sys1(_subproblem.getSystem(getParam<VariableName>(nl_vnames[0])));
  _ndisp = nl_vnames.size();
  // for (unsigned int i = 0; i < _ndisp; ++i)
  //   _disp_num[i] = _subproblem.getVariable(_tid,
  //                                          parameters.
  //                                          get<VariableName>(nl_vnames[i]),
  //                                          Moose::VarKindType::VAR_ANY,
  //                                          Moose::VarFieldType::VAR_FIELD_STANDARD)
  //                                          .number();
  _disp_num[0] = _var_0;
  _disp_num[1] = _var_1;
  _disp_num[2] = _var_2;
}

void
AveragePointSeparation::execute()
{
  unsigned int num_point = _first_point.size();
  _value = 0.;
  Real sq_diff;
  for (unsigned int i = 0; i < num_point; ++i)
  {
    sq_diff = 0;
    for (unsigned int j = 0; j < _ndisp; ++j)
    {
      sq_diff += pow(_system.point_value(_disp_num[j], _first_point[i], false) -
                     _system.point_value(_disp_num[j], _last_point[i], false), 2.0);

    }
    _value += std::sqrt(sq_diff);
  }
  _value = _value/num_point;
}

Real
AveragePointSeparation::getValue()
{
  return _value;
}
