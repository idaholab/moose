/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMass.h"

template <>
InputParameters
validParams<RichardsMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
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
