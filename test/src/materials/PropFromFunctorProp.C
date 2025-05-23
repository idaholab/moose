//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PropFromFunctorProp.h"

registerMooseObject("MooseTestApp", PropFromFunctorProp);

InputParameters
PropFromFunctorProp::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<MooseFunctorName>("vector_functor", "A vector functor");
  params.addRequiredParam<MaterialPropertyName>("vector_prop", "The vector property to declare");
  return params;
}

PropFromFunctorProp::PropFromFunctorProp(const InputParameters & params)
  : Material(params),
    _vector_functor(getFunctor<ADRealVectorValue>("vector_functor")),
    _vector_prop(declareADProperty<RealVectorValue>("vector_prop"))
{
}

void
PropFromFunctorProp::computeQpProperties()
{
  _vector_prop[_qp] = _vector_functor(Moose::ElemQpArg{_current_elem, _qp, _qrule, _q_point[_qp]},
                                      Moose::currentState());
}
