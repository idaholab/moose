//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckActiveMatProp.h"
#include "MooseApp.h"
#include "CheckActiveMatPropProblem.h"
#include "MaterialData.h"

registerMooseObject("MooseTestApp", CheckActiveMatProp);

InputParameters
CheckActiveMatProp::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the material property");
  return params;
}

CheckActiveMatProp::CheckActiveMatProp(const InputParameters & parameters)
  : AuxKernel(parameters),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _problem(_app.feProblem()),
    _data(boundaryRestricted() ? _problem.getMaterialData(Moose::BOUNDARY_MATERIAL_DATA, _tid)
                               : _problem.getMaterialData(Moose::BLOCK_MATERIAL_DATA, _tid)),
    _prop_id(_data.getPropertyId(_prop_name))
{
}

Real
CheckActiveMatProp::computeValue()
{
  mooseAssert(dynamic_cast<CheckActiveMatPropProblem *>(&_problem),
              "We don't want undefined behavior");
  return static_cast<CheckActiveMatPropProblem &>(_problem).getActiveMaterialProperties(_tid).count(
             _prop_id) > 0;
}
