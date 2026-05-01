//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SharpInterfaceFluxBranchConsistencyError.h"

#include "FaceInfo.h"
#include "MooseMesh.h"
#include "SharpInterfaceRhieChowMassFlux.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", SharpInterfaceFluxBranchConsistencyError);

InputParameters
SharpInterfaceFluxBranchConsistencyError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum quantity("predictor_operator pressure_correction total", "total");
  MooseEnum metric("l2 max_abs", "l2");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object",
                                          "The sharp-interface Rhie-Chow user object.");
  params.addRequiredParam<MooseEnum>("quantity",
                                     quantity,
                                     "Which face-flux branch to compare against its "
                                     "corresponding cell-velocity reconstruction.");
  params.addParam<MooseEnum>("metric",
                             metric,
                             "Whether to compute a face-measure-weighted L2 norm or a max "
                             "absolute mismatch.");
  params.addParam<MooseFunctorName>(
      "density", "rho_mixture", "The density functor used to build the mass flux.");
  params.addParam<MooseFunctorName>("vel_x",
                                    "vel_x",
                                    "The x-velocity functor for the total-branch comparison.");
  params.addParam<MooseFunctorName>("vel_y",
                                    "",
                                    "The y-velocity functor for the total-branch comparison.");
  params.addParam<MooseFunctorName>("vel_z",
                                    "",
                                    "The z-velocity functor for the total-branch comparison.");
  params.addClassDescription("Computes a weighted internal-face consistency error for the "
                             "sharp-interface face-flux chain, split into predictor-operator, "
                             "pressure-correction, and total branches.");
  return params;
}

SharpInterfaceFluxBranchConsistencyError::SharpInterfaceFluxBranchConsistencyError(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _rhie_chow(getUserObject<SharpInterfaceRhieChowMassFlux>("rhie_chow_user_object")),
    _quantity(getParam<MooseEnum>("quantity") == "predictor_operator"
                  ? Quantity::PredictorOperator
                  : getParam<MooseEnum>("quantity") == "pressure_correction"
                        ? Quantity::PressureCorrection
                        : Quantity::Total),
    _metric(getParam<MooseEnum>("metric") == "max_abs" ? Metric::MaxAbs : Metric::L2),
    _rho(getFunctor<Real>("density")),
    _vel_x(isParamValid("vel_x") && !getParam<MooseFunctorName>("vel_x").empty()
               ? &getFunctor<Real>("vel_x")
               : nullptr),
    _vel_y(isParamValid("vel_y") && !getParam<MooseFunctorName>("vel_y").empty()
               ? &getFunctor<Real>("vel_y")
               : nullptr),
    _vel_z(isParamValid("vel_z") && !getParam<MooseFunctorName>("vel_z").empty()
               ? &getFunctor<Real>("vel_z")
               : nullptr),
    _value(0.0)
{
}

void
SharpInterfaceFluxBranchConsistencyError::initialize()
{
  _value = 0.0;
}

void
SharpInterfaceFluxBranchConsistencyError::execute()
{
  const auto time_arg = determineState();

  for (const auto * fi : _mesh.faceInfo())
  {
    if (!fi || !fi->elemPtr() || !fi->neighborPtr())
      continue;

    const Moose::FaceArg face_arg{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, fi->elemPtr(), nullptr};

    const Real face_rho = MetaPhysicL::raw_value(_rho(face_arg, time_arg));
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    RealVectorValue face_velocity;

    if (_quantity == Quantity::PredictorOperator)
    {
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_velocity(0),
                             _rhie_chow.predictorVelocityComponent(elem_info, 0),
                             _rhie_chow.predictorVelocityComponent(neighbor_info, 0),
                             *fi,
                             true);

      if (_mesh.dimension() > 1)
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_velocity(1),
                               _rhie_chow.predictorVelocityComponent(elem_info, 1),
                               _rhie_chow.predictorVelocityComponent(neighbor_info, 1),
                               *fi,
                               true);

      if (_mesh.dimension() > 2)
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_velocity(2),
                               _rhie_chow.predictorVelocityComponent(elem_info, 2),
                               _rhie_chow.predictorVelocityComponent(neighbor_info, 2),
                               *fi,
                               true);
    }
    else if (_quantity == Quantity::PressureCorrection)
    {
      const RealVectorValue elem_delta =
          _rhie_chow.pressureCoupledCellVelocityDelta(elem_info, time_arg);
      const RealVectorValue neighbor_delta =
          _rhie_chow.pressureCoupledCellVelocityDelta(neighbor_info, time_arg);

      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_velocity(0),
                             elem_delta(0),
                             neighbor_delta(0),
                             *fi,
                             true);

      if (_mesh.dimension() > 1)
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_velocity(1),
                               elem_delta(1),
                               neighbor_delta(1),
                               *fi,
                               true);

      if (_mesh.dimension() > 2)
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_velocity(2),
                               elem_delta(2),
                               neighbor_delta(2),
                               *fi,
                               true);
    }
    else
    {
      mooseAssert(_vel_x, "The total-branch consistency metric requires vel_x.");

      const Real elem_u = MetaPhysicL::raw_value((*_vel_x)(makeElemArg(fi->elemPtr()), time_arg));
      const Real neighbor_u =
          MetaPhysicL::raw_value((*_vel_x)(makeElemArg(fi->neighborPtr()), time_arg));
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_velocity(0),
                             elem_u,
                             neighbor_u,
                             *fi,
                             true);

      if (_vel_y)
      {
        const Real elem_v =
            MetaPhysicL::raw_value((*_vel_y)(makeElemArg(fi->elemPtr()), time_arg));
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
        const Real elem_w =
            MetaPhysicL::raw_value((*_vel_z)(makeElemArg(fi->elemPtr()), time_arg));
        const Real neighbor_w =
            MetaPhysicL::raw_value((*_vel_z)(makeElemArg(fi->neighborPtr()), time_arg));
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_velocity(2),
                               elem_w,
                               neighbor_w,
                               *fi,
                               true);
      }
    }

    const Real reconstructed_flux = face_rho * (face_velocity * fi->normal());
    const Real stored_flux = _quantity == Quantity::PredictorOperator
                                 ? _rhie_chow.predictorOperatorFaceMassFlux(*fi, time_arg)
                             : _quantity == Quantity::PressureCorrection
                                 ? _rhie_chow.pressureCoupledWritebackMassFlux(*fi)
                                 : _rhie_chow.rawRhieChowMassFlux(*fi);
    const Real diff = stored_flux - reconstructed_flux;

    if (_metric == Metric::MaxAbs)
      _value = std::max(_value, std::abs(diff));
    else
      _value += diff * diff * fi->faceArea() * fi->faceCoord();
  }
}

void
SharpInterfaceFluxBranchConsistencyError::finalize()
{
  if (_metric == Metric::MaxAbs)
    _communicator.max(_value);
  else
  {
    _communicator.sum(_value);
    _value = std::sqrt(_value);
  }
}

Real
SharpInterfaceFluxBranchConsistencyError::getValue() const
{
  return _value;
}
