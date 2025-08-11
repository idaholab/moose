//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledValueDiffusionKernel.h"

registerMooseObject("MooseTestApp", FVCoupledValueDiffusionKernel);

InputParameters
FVCoupledValueDiffusionKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params += FVDiffusionInterpolationInterface::validParams();
  params.addClassDescription("Computes residual for a coupled variable time the "
                             "diffusion operator for finite volume method.");
  params.addRequiredCoupledVar("v", "The coupled variable.");

  MooseEnum coeff_interp_method("average harmonic", "harmonic");
  params.addParam<MooseEnum>(
      "coeff_interp_method",
      coeff_interp_method,
      "Switch that can select face interpolation method for diffusion coefficients.");

  params.set<unsigned short>("ghost_layers") = 2;

  // We add the relationship manager here, this will select the right number of
  // ghosting layers depending on the chosen interpolation method
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      { FVRelationshipManagerInterface::setRMParamsDiffusion(obj_params, rm_params, 3); });

  return params;
}

FVCoupledValueDiffusionKernel::FVCoupledValueDiffusionKernel(const InputParameters & params)
  : FVFluxKernel(params), 
    FVDiffusionInterpolationInterface(params), 
    _v_elem(adCoupledValue("v")), 
    _v_neighbor(adCoupledNeighborValue("v")),
    _coeff_interp_method(
        Moose::FV::selectInterpolationMethod(getParam<MooseEnum>("coeff_interp_method")))
{
}

ADReal
FVCoupledValueDiffusionKernel::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  auto dudn = gradUDotNormal(state, _correct_skewness);

  ADReal v;
  interpolate(
      Moose::FV::InterpMethod::Average, v, _v_elem[_qp], _v_neighbor[_qp], *_face_info, true);

  return v * dudn;
}
