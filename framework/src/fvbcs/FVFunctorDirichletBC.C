//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorDirichletBC.h"

registerMooseObject("MooseApp", FVFunctorDirichletBC);
registerMooseObject("MooseApp", FVADFunctorDirichletBC);

template <bool is_ad>
InputParameters
FVFunctorDirichletBCTempl<is_ad>::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Uses the value of a functor to set a Dirichlet boundary value.");
  params.addRequiredParam<MooseFunctorName>(
      "functor", "The name of the functor whose value is imposed on the boundary");
  return params;
}

template <bool is_ad>
FVFunctorDirichletBCTempl<is_ad>::FVFunctorDirichletBCTempl(const InputParameters & parameters)
  : FVDirichletBCBase(parameters), _functor(getFunctor<GenericReal<is_ad>>("functor"))
{
}

template <bool is_ad>
ADReal
FVFunctorDirichletBCTempl<is_ad>::boundaryValue(const FaceInfo & fi) const
{
  return _functor(singleSidedFaceArg(&fi), determineState());
}
