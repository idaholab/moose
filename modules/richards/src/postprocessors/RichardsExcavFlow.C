//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsExcavFlow.h"
#include "Function.h"
#include "Material.h"

registerMooseObject("RichardsApp", RichardsExcavFlow);

InputParameters
RichardsExcavFlow::validParams()
{
  InputParameters params = SideIntegralVariablePostprocessor::validParams();
  params.addRequiredParam<FunctionName>(
      "excav_geom_function",
      "The function describing the excavation geometry (type RichardsExcavGeom)");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addClassDescription("Records total flow INTO an excavation (if quantity is positive then "
                             "flow has occured from rock into excavation void)");
  return params;
}

RichardsExcavFlow::RichardsExcavFlow(const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(coupled("variable"))),

    _flux(getMaterialProperty<std::vector<RealVectorValue>>("flux")),

    _func(getFunction("excav_geom_function"))
{
}

Real
RichardsExcavFlow::computeQpIntegral()
{
  return -_func.value(_t, _q_point[_qp]) * _normals[_qp] * _flux[_qp][_pvar] * _dt;
}
