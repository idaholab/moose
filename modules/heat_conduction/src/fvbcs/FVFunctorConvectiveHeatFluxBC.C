//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorConvectiveHeatFluxBC.h"
#include "Function.h"

registerMooseObject("HeatConductionApp", FVFunctorConvectiveHeatFluxBC);

InputParameters
FVFunctorConvectiveHeatFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by functors.");
  params.addRequiredParam<MooseFunctorName>("T_wall", "Functor for wall temperature");
  params.addRequiredParam<MooseFunctorName>("T_infinity", "Functor for far-field temperature");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Functor for heat transfer coefficient");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");

  return params;
}

FVFunctorConvectiveHeatFluxBC::FVFunctorConvectiveHeatFluxBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _T_wall(getFunctor<ADReal>("T_wall")),
    _T_infinity(getFunctor<ADReal>("T_infinity")),
    _htc(getFunctor<ADReal>("heat_transfer_coefficient")),
    _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
FVFunctorConvectiveHeatFluxBC::computeQpResidual()
{
  if (_is_solid)
    return -_htc(singleSidedFaceArg()) *
           (_T_infinity(singleSidedFaceArg()) - _T_wall(singleSidedFaceArg()));
  else
    return _htc(singleSidedFaceArg()) *
           (_T_infinity(singleSidedFaceArg()) - _T_wall(singleSidedFaceArg()));
}
