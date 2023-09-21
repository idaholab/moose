//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumPressureFlux.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumPressureFlux);

InputParameters
INSFVMomentumPressureFlux::validParams()
{
  auto params = FVFluxKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription("Momentum pressure term eps grad_P, as a flux kernel "
                             "using the divergence theoreom, in the  "
                             "incompressible Navier-Stokes momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure");
  return params;
}

INSFVMomentumPressureFlux::INSFVMomentumPressureFlux(const InputParameters & params)
  : FVFluxKernel(params), INSFVMomentumResidualObject(*this), _p(getFunctor<ADReal>(NS::pressure))
{
  if (!dynamic_cast<INSFVVelocityVariable *>(&_var))
    mooseError("INSFVMomentumPressureFlux may only be used with a Navier-Stokes velocity, "
               "of variable type INSFVSuperficialVelocityVariable.");
}

ADReal
INSFVMomentumPressureFlux::computeQpResidual()
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
    eps_p_interface = epsilon()(elem, state) * _p(singleSidedFaceArg(), state);
  else
  {
    const auto * neighbor_ptr = use_elem ? _face_info->neighborPtr() : &_face_info->elem();
    const auto & neighbor = makeElemArg(neighbor_ptr);

    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           eps_p_interface,
                           epsilon()(elem, state) * _p(elem, state),
                           epsilon()(neighbor, state) * _p(neighbor, state),
                           *_face_info,
                           true);
  }

  return eps_p_interface * _normal(_index);
}
