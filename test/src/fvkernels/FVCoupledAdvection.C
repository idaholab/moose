//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledAdvection.h"

registerADMooseObject("MooseTestApp", FVCoupledAdvection);

InputParameters
FVCoupledAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Residual contribution from advection operator for finite volume method "
      "with the gradient of the coupled varaible as the advection velocity.");
  params.addRequiredCoupledVar("v", "The coupled variable to take the gradient of.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

FVCoupledAdvection::FVCoupledAdvection(const InputParameters & params)
  : FVFluxKernel(params), _v_var(*getFieldVar("v", 0))
{
  const bool need_more_ghosting =
      Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
  if (need_more_ghosting && _tid == 0)
  {
    adjustRMGhostLayers(std::max((unsigned short)(2), _pars.get<unsigned short>("ghost_layers")));

    // If we need more ghosting, then we are a second-order nonlinear limiting scheme whose stencil
    // is liable to change upon wind-direction change. Consequently we need to tell our problem that
    // it's ok to have new nonzeros which may crop-up after PETSc has shrunk the matrix memory
    getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
        ->setErrorOnJacobianNonzeroReallocation(false);
  }
}

ADReal
FVCoupledAdvection::computeQpResidual()
{
  using namespace Moose::FV;
  const auto state = determineState();

  ADRealVectorValue velocity = adCoupledGradientFace(_v_var.name(), *_face_info, state);

  const bool elem_is_upwind = velocity * _normal >= 0;
  const auto face =
      makeFace(*_face_info, Moose::FV::limiterType(_advected_interp_method), elem_is_upwind);
  ADReal u_interface = _var(face, determineState());

  return _normal * velocity * u_interface;
}
