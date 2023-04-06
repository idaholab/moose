//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorNeumannBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVFunctorNeumannBC);

InputParameters
FVFunctorNeumannBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Neumann boundary condition for the finite volume method.");
  params.addParam<MooseFunctorName>("factor",
                                    1.,
                                    "A factor in the form of a functor for multiplying the "
                                    "function. This could be useful for flipping "
                                    "the sign of the function for example based off the normal");
  params.addRequiredParam<MooseFunctorName>("functor",
                                            "The value of the flux crossing the boundary.");
  return params;
}

FVFunctorNeumannBC::FVFunctorNeumannBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _functor(getFunctor<ADReal>("functor")),
    _factor(getFunctor<ADReal>("factor"))
{
}

ADReal
FVFunctorNeumannBC::computeQpResidual()
{
  return -_factor(singleSidedFaceArg(), determineState()) *
         _functor(singleSidedFaceArg(), determineState());
}
