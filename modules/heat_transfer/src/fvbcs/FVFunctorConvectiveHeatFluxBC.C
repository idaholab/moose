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
      "transfer coefficient given by functors.");
  params.addRequiredParam<MooseFunctorName>("T_solid", "Functor for wall temperature");
  params.addRequiredParam<MooseFunctorName>("T_bulk", "Functor for far-field temperature");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Functor for heat transfer coefficient");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");

  return params;
}

FVFunctorConvectiveHeatFluxBC::FVFunctorConvectiveHeatFluxBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _T_solid(getFunctor<ADReal>("T_solid")),
    _T_bulk(getFunctor<ADReal>("T_bulk")),
    _htc(getFunctor<ADReal>("heat_transfer_coefficient")),
    _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
FVFunctorConvectiveHeatFluxBC::computeQpResidual()
{
  // Allow the functors to pick their side evaluation since either T_bulk or T_solid is likely not
  // defined on this boundary condition's side
  const Moose::FaceArg face{
      _face_info, Moose::FV::LimiterType::CentralDifference, true, false, nullptr};
  const auto flux = _htc(face, determineState()) *
                    (_T_bulk(face, determineState()) - _T_solid(face, determineState()));
  if (_is_solid)
    return -flux;
  else
    return flux;
}
