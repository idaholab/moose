//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SharpInterfaceVolumetricFluxConsistencyError.h"

#include "FaceInfo.h"
#include "MooseMesh.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", SharpInterfaceVolumetricFluxConsistencyError);

InputParameters
SharpInterfaceVolumetricFluxConsistencyError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<MooseFunctorName>(
      "final_face_flux", "corrected_face_phi", "The final volumetric face-flux functor.");
  params.addRequiredParam<MooseFunctorName>("vel_x", "The x-velocity functor.");
  params.addParam<MooseFunctorName>("vel_y", "", "The y-velocity functor.");
  params.addParam<MooseFunctorName>("vel_z", "", "The z-velocity functor.");
  params.addClassDescription("Computes a face-measure-weighted L2 norm of the mismatch between "
                             "the stored sharp-interface volumetric face flux and the flux "
                             "reconstructed from the current cell-centered velocity field.");
  return params;
}

SharpInterfaceVolumetricFluxConsistencyError::SharpInterfaceVolumetricFluxConsistencyError(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _final_phi(getFunctor<Real>("final_face_flux")),
    _vel_x(getFunctor<Real>("vel_x")),
    _vel_y(isParamValid("vel_y") && !getParam<MooseFunctorName>("vel_y").empty()
               ? &getFunctor<Real>("vel_y")
               : nullptr),
    _vel_z(isParamValid("vel_z") && !getParam<MooseFunctorName>("vel_z").empty()
               ? &getFunctor<Real>("vel_z")
               : nullptr),
    _error(0.0)
{
}

void
SharpInterfaceVolumetricFluxConsistencyError::initialize()
{
  _error = 0.0;
}

void
SharpInterfaceVolumetricFluxConsistencyError::execute()
{
  const auto time_arg = determineState();

  for (const auto * fi : _mesh.faceInfo())
  {
    if (!fi || !fi->elemPtr() || !fi->neighborPtr())
      continue;

    const auto face_arg =
        Moose::FaceArg{fi, Moose::FV::LimiterType::CentralDifference, true, false, fi->elemPtr(), nullptr};

    RealVectorValue face_velocity;

    const Real elem_u = MetaPhysicL::raw_value(_vel_x(makeElemArg(fi->elemPtr()), time_arg));
    const Real neighbor_u = MetaPhysicL::raw_value(_vel_x(makeElemArg(fi->neighborPtr()), time_arg));
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           face_velocity(0),
                           elem_u,
                           neighbor_u,
                           *fi,
                           true);

    if (_vel_y)
    {
      const Real elem_v = MetaPhysicL::raw_value((*_vel_y)(makeElemArg(fi->elemPtr()), time_arg));
      const Real neighbor_v =
          MetaPhysicL::raw_value((*_vel_y)(makeElemArg(fi->neighborPtr()), time_arg));
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_velocity(1),
                             elem_v,
                             neighbor_v,
                             *fi,
                             true);
    }

    if (_vel_z)
    {
      const Real elem_w = MetaPhysicL::raw_value((*_vel_z)(makeElemArg(fi->elemPtr()), time_arg));
      const Real neighbor_w =
          MetaPhysicL::raw_value((*_vel_z)(makeElemArg(fi->neighborPtr()), time_arg));
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_velocity(2),
                             elem_w,
                             neighbor_w,
                             *fi,
                             true);
    }

    const Real stored_phi = MetaPhysicL::raw_value(_final_phi(face_arg, time_arg));
    const Real reconstructed_phi = face_velocity * fi->normal();
    const Real diff = stored_phi - reconstructed_phi;
    _error += diff * diff * fi->faceArea() * fi->faceCoord();
  }
}

void
SharpInterfaceVolumetricFluxConsistencyError::finalize()
{
  _communicator.sum(_error);
  _error = std::sqrt(_error);
}

Real
SharpInterfaceVolumetricFluxConsistencyError::getValue() const
{
  return _error;
}
