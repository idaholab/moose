//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OnTheFlyTagVectorAux.h"

registerMooseObject("MooseTestApp", OnTheFlyTagVectorAux);

InputParameters
OnTheFlyTagVectorAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addParam<unsigned int>(
      "component", 0, "The component at which to index the coupled-in variables.");
  return params;
}

OnTheFlyTagVectorAux::OnTheFlyTagVectorAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledVectorTagValues("v", "vector_tag")),
    _component(this->template getParam<unsigned int>("component"))
{
}

Real
OnTheFlyTagVectorAux::computeValue()
{
  return (*_v[_component])[_qp];
}
