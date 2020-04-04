//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMass.h"

registerMooseObject("RichardsApp", RichardsMass);

InputParameters
RichardsMass::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addClassDescription("Returns the mass in a region.");
  return params;
}

RichardsMass::RichardsMass(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(coupled("variable"))),

    _mass(getMaterialProperty<std::vector<Real>>("mass"))
{
}

Real
RichardsMass::computeQpIntegral()
{
  return _mass[_qp][_pvar];
}
