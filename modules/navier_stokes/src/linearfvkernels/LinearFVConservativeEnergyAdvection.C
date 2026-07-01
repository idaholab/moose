//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "LinearFVConservativeEnergyAdvection.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", LinearFVConservativeEnergyAdvection);

InputParameters
LinearFVConservativeEnergyAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Advects conserved thermal energy E = rho * cp * T with a "
                             "VOF-consistent rho * cp face flux.");
  params.addRequiredParam<MooseFunctorName>("rho_cp_face_flux",
                                            "Face flux of rho * cp used to advect temperature.");
  params.addRequiredParam<MooseFunctorName>("rho_cp", "Cell-centered rho * cp functor.");
  params.addParam<bool>(
      "face_flux_is_integrated", false, "Whether rho_cp_face_flux already includes the face area.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVConservativeEnergyAdvection::LinearFVConservativeEnergyAdvection(
    const InputParameters & params)
  : LinearFVFluxKernel(params),
    _rho_cp_face_flux(getFunctor<Real>("rho_cp_face_flux")),
    _rho_cp(getFunctor<Real>("rho_cp")),
    _face_flux_is_integrated(getParam<bool>("face_flux_is_integrated")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_flux(0.0)
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

Real
LinearFVConservativeEnergyAdvection::elemInverseRhoCp() const
{
  return 1.0 / _rho_cp(makeElemArg(_current_face_info->elemPtr()), determineState());
}

Real
LinearFVConservativeEnergyAdvection::neighborInverseRhoCp() const
{
  return 1.0 / _rho_cp(makeElemArg(_current_face_info->neighborPtr()), determineState());
}

Real
LinearFVConservativeEnergyAdvection::singleSidedInverseRhoCp() const
{
  const auto elem = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                        ? _current_face_info->elemPtr()
                        : _current_face_info->neighborPtr();
  return 1.0 / _rho_cp(makeElemArg(elem), determineState());
}

Real
LinearFVConservativeEnergyAdvection::computeElemMatrixContribution()
{
  return _advected_interp_coeffs.first * elemInverseRhoCp() * _face_flux * _current_face_area;
}

Real
LinearFVConservativeEnergyAdvection::computeNeighborMatrixContribution()
{
  return _advected_interp_coeffs.second * neighborInverseRhoCp() * _face_flux * _current_face_area;
}

Real
LinearFVConservativeEnergyAdvection::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVConservativeEnergyAdvection::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVConservativeEnergyAdvection::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;
  return adv_bc->computeBoundaryValueMatrixContribution() * singleSidedInverseRhoCp() * factor *
         _face_flux * _current_face_area;
}

Real
LinearFVConservativeEnergyAdvection::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;
  return -adv_bc->computeBoundaryValueRHSContribution() * factor * _face_flux * _current_face_area;
}

void
LinearFVConservativeEnergyAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  _face_flux = _rho_cp_face_flux(functorFaceArg(_rho_cp_face_flux, *face_info), determineState());
  if (_face_flux_is_integrated)
    _face_flux = _current_face_area > 0.0 ? _face_flux / _current_face_area : 0.0;

  _advected_interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _face_flux);
}
