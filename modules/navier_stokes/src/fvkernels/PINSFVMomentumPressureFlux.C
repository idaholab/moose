//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressureFlux.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressureFlux);

InputParameters
PINSFVMomentumPressureFlux::validParams()
{
  auto params = FVFluxKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Momentum pressure term eps grad_P, as a flux kernel "
                             "using the divergence theoreom, in the porous media "
                             "incompressible Navier-Stokes momentum equation. This kernel "
                             "is also executed on boundaries.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity functor");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>("momentum_component",
                                     momentum_component,
                                     "The component of the momentum equation that this kernel "
                                     "applies to.");
  params.set<bool>("force_boundary_execution") = true;
  return params;
}

PINSFVMomentumPressureFlux::PINSFVMomentumPressureFlux(const InputParameters & params)
  : FVFluxKernel(params),
    INSFVMomentumResidualObject(*this),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _p(getFunctor<ADReal>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressureFlux may only be used with a superficial velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressureFlux::computeQpResidual()
{
  ADReal eps_p_interface;
  // Momentum and porosity domains should match
  const auto & face_type = _face_info->faceType(_var.name());
  const bool use_elem = (face_type == FaceInfo::VarFaceNeighbors::ELEM) ||
                        (face_type == FaceInfo::VarFaceNeighbors::BOTH);

  const auto * const elem_ptr = use_elem ? &_face_info->elem() : _face_info->neighborPtr();
  const auto & elem = makeElemArg(elem_ptr);
  const auto state = determineState();

  if (onBoundary(*_face_info))
    eps_p_interface = _eps(elem, state) * _p(singleSidedFaceArg(), state);
  else
  {
    const auto * neighbor_ptr = use_elem ? _face_info->neighborPtr() : &_face_info->elem();
    const auto & neighbor = makeElemArg(neighbor_ptr);

    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           eps_p_interface,
                           _eps(elem, state) * _p(elem, state),
                           _eps(neighbor, state) * _p(neighbor, state),
                           *_face_info,
                           true);
  }

  return eps_p_interface * _normal(_index);
}
