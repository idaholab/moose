//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "SolutionAuxMisoBnds.h"
#include "SolutionUserObject.h"
#include "BndsCalculator.h"

registerMooseObject("labmouseApp", SolutionAuxMisoBnds);

InputParameters
SolutionAuxMisoBnds::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Creates fields by using information from a SolutionUserObject.");
  params.addRequiredParam<UserObjectName>("solution", "The name of the SolutionUserObject");
  params.addParam<std::string>("from_gb_type",
                               "The name of the variable to extract from the file");
  params.addParam<bool>(
      "direct",
      false,
      "If true the meshes must be the same and then the values are simply copied over.");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<Real>("gb_type_order",
                                "The grain boundary type to calculate bnds parameter");
  return params;
}

SolutionAuxMisoBnds::SolutionAuxMisoBnds(const InputParameters & parameters)
  : AuxKernel(parameters),
    _solution_object(getUserObject<SolutionUserObject>("solution")),
    _direct(getParam<bool>("direct")),
    _gb_type_order(getParam<Real>("gb_type_order")),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v"))
{
}

void
SolutionAuxMisoBnds::initialSetup()
{
  // If 'from_variable' is supplied, use the value
  if (isParamValid("from_gb_type"))
    _var_name_gb_type = getParam<std::string>("from_gb_type");

  // If not, get the value from the SolutionUserObject
  else
  {
    // Get all the variables from the SolutionUserObject
    const std::vector<std::string> & vars = _solution_object.variableNames();

    // If there are more than one, throw an error
    if (vars.size() > 1)
      mooseError("The SolutionUserObject contains multiple variables, please specifiy the desired "
                 "variables in the input file with 'from_variable' parameter.");

    // Define the variable
    _var_name_gb_type = vars[0];
  }
}

Real
SolutionAuxMisoBnds::computeValue()
{
  // The value to output
  Real output_gb_type;

  // _direct=true, extract the values using the dof
  if (_direct)
  {
    if (isNodal())
      output_gb_type = _solution_object.directValue(_current_node, _var_name_gb_type);
    else
      output_gb_type = _solution_object.directValue(_current_elem, _var_name_gb_type);
  }
  // _direct=false, extract the values using time and point
  else
  {
    if (isNodal())
      output_gb_type = _solution_object.pointValue(_t, *_current_node, _var_name_gb_type);
    else
      output_gb_type = _solution_object.pointValue(_t, _current_elem->vertex_average(), _var_name_gb_type);
  }

  // generate different gb_bnds
  auto gb_type_shift = abs(output_gb_type-_gb_type_order);
  if (abs(output_gb_type-0) < 1e-3)
    return 1.0;
  else if (gb_type_shift < 1e-3)
    return BndsCalculator::computeBndsVariable(_vals, _qp);
  else
    return gb_type_shift+(1-gb_type_shift)*BndsCalculator::computeBndsVariable(_vals, _qp);

}
