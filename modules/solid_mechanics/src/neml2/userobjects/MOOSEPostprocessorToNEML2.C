//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEPostprocessorToNEML2.h"

registerMooseObject("SolidMechanicsApp", MOOSEPostprocessorToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldPostprocessorToNEML2);

template <unsigned int state>
InputParameters
MOOSEPostprocessorToNEML2Templ<state>::validParams()
{
  auto params = MOOSEToNEML2Unbatched::validParams();
  params.addClassDescription(NEML2Utils::docstring(
      "Gather a MOOSE postprocessor value for insertion into the specified input variable or "
      "model parameter of a NEML2 model."));
  params.addRequiredParam<PostprocessorName>(
      "from_moose", NEML2Utils::docstring("MOOSE postprocessor to read from"));
  return params;
}

template <>
MOOSEPostprocessorToNEML2Templ<0>::MOOSEPostprocessorToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2Unbatched(params)
#ifdef NEML2_ENABLED
    ,
    _moose_pp(getPostprocessorValue("from_moose"))
#endif
{
}

template <>
MOOSEPostprocessorToNEML2Templ<1>::MOOSEPostprocessorToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2Unbatched(params)
#ifdef NEML2_ENABLED
    ,
    _moose_pp(getPostprocessorValueOld("from_moose"))
#endif
{
}

#ifdef NEML2_ENABLED
template <unsigned int state>
neml2::Tensor
MOOSEPostprocessorToNEML2Templ<state>::gatheredData() const
{
  return neml2::Scalar::full(_moose_pp);
}
#endif

template class MOOSEPostprocessorToNEML2Templ<0>;
template class MOOSEPostprocessorToNEML2Templ<1>;
