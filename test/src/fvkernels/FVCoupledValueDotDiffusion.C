//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledValueDotDiffusion.h"

registerMooseObject("MooseApp", FVCoupledValueDotDiffusion);

InputParameters
FVCoupledValueDotDiffusion::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes residual for a coupled variable time the "
                             "diffusion operator for finite volume method.");
  params.addRequiredCoupledVar("v", "The coupled variable.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVCoupledValueDotDiffusion::FVCoupledValueDotDiffusion(const InputParameters & params)
  : FVFluxKernel(params), _v_elem(adCoupledValue("v")), _v_neighbor(adCoupledNeighborValue("v"))
{
}

ADReal
FVCoupledValueDotDiffusion::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  auto dudn = gradUDotNormal(state);

  // Eventually, it will be nice to offer automatic-switching triggered by
  // input parameters to change between different interpolation methods for
  // this.
  ADReal v;
  interpolate(
      Moose::FV::InterpMethod::Average, v, _v_elem[_qp], _v_neighbor[_qp], *_face_info, true);

  return v * dudn;
}
