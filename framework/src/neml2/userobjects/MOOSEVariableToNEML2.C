//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEVariableToNEML2.h"

registerMooseObject("MooseApp", MOOSEVariableToNEML2);
registerMooseObject("MooseApp", MOOSEOldVariableToNEML2);

template <unsigned int state>
InputParameters
MOOSEVariableToNEML2Templ<state>::validParams()
{
  auto params = MOOSEToNEML2Batched::validParams();
  params.addClassDescription("Gather a MOOSE variable for insertion into the specified input or "
                             "model parameter of a NEML2 model.");
  params.addRequiredCoupledVar("from_moose", "MOOSE variable to read from");
  return params;
}

template <unsigned int state>
MOOSEVariableToNEML2Templ<state>::MOOSEVariableToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2Batched<Real>(params)
#ifdef NEML2_ENABLED
    ,
    _moose_variable(state == 0 ? this->coupledValue("from_moose")
                               : this->coupledValueOld("from_moose")),
    _moose_variable_neighbor(state == 0 ? this->coupledNeighborValue("from_moose")
                                        : this->coupledNeighborValueOld("from_moose"))
#endif
{
  static_assert(state < 2,
                "MOOSEVariableToNEML2Tmpl supports only state=0 (current) or state=1 (old)");
}

template class MOOSEVariableToNEML2Templ<0>;
template class MOOSEVariableToNEML2Templ<1>;
