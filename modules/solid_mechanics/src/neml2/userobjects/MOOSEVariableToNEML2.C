//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEVariableToNEML2.h"

registerMooseObject("SolidMechanicsApp", MOOSEVariableToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldVariableToNEML2);

template <unsigned int state>
InputParameters
MOOSEVariableToNEML2Templ<state>::validParams()
{
  auto params = MOOSEToNEML2Batched::validParams();
  params.addClassDescription(
      NEML2Utils::docstring("Gather a MOOSE variable for insertion into the specified input or "
                            "model parameter of a NEML2 model."));
  params.addRequiredCoupledVar("from_moose", NEML2Utils::docstring("MOOSE variable to read from"));
  return params;
}

template <>
MOOSEVariableToNEML2Templ<0>::MOOSEVariableToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2Batched(params)
#ifdef NEML2_ENABLED
    ,
    _moose_variable(coupledValue("from_moose"))
#endif
{
}

template <>
MOOSEVariableToNEML2Templ<1>::MOOSEVariableToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2Batched(params)
#ifdef NEML2_ENABLED
    ,
    _moose_variable(coupledValueOld("from_moose"))
#endif
{
}

template class MOOSEVariableToNEML2Templ<0>;
template class MOOSEVariableToNEML2Templ<1>;
