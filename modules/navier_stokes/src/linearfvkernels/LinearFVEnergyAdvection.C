//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVEnergyAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NS.h"
#include "PorousRhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", LinearFVEnergyAdvection);

InputParameters
LinearFVEnergyAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for the energy e.g. h=int(cp dT). A user may still "
                             "override what quantity is advected, but the default is enthalpy.");
  MooseEnum advected_quantity("enthalpy temperature", "enthalpy");
  params.addParam<MooseEnum>("advected_quantity", advected_quantity, "The advected quantity");
  params.addParam<Real>("cp", "Constant specific heat value");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params.addParam<bool>(
      "subtract_continuity_error",
      false,
      "Subtract the local discrete mass-flux divergence from the conservative advection operator. "
      "This advective-form correction preserves constant enthalpy/temperature fields when the "
      "segregated face flux is not exactly divergence-free.");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVEnergyAdvection::LinearFVEnergyAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _advected_quantity(getParam<MooseEnum>("advected_quantity").getEnum<AdvectedQuantityEnum>()),
    _cp(isParamValid("cp") ? getParam<Real>("cp") : 1.0),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_mass_flux(0.0),
    _subtract_continuity_error(getParam<bool>("subtract_continuity_error"))
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");

  if (isParamValid("cp") && _advected_quantity == AdvectedQuantityEnum::ENTHALPY)
    paramError("cp", "cp should not be specified for enthalpy advection");

  if (!isParamValid("cp") && _advected_quantity == AdvectedQuantityEnum::TEMPERATURE)
    paramError("cp", "cp should be specified for temperature advection");

  if (dynamic_cast<const PorousRhieChowMassFlux *>(&_mass_flux_provider) &&
      _advected_quantity != AdvectedQuantityEnum::ENTHALPY)
    paramError("advected_quantity",
               "LinearFVEnergyAdvection only supports specific enthalpy advection when used with "
               "PorousRhieChowMassFlux.");
}

void
LinearFVEnergyAdvection::addMatrixContribution()
{
  LinearFVFluxKernel::addMatrixContribution();

  if (!_subtract_continuity_error)
    return;

  const Real continuity_correction = _cp * _face_mass_flux * _current_face_area;

  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH ||
      _current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
  {
    if (hasBlocks(_current_face_info->elemInfo()->subdomain_id()))
    {
      const auto dof_id_elem = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
      for (auto & matrix : _matrices)
        (*matrix).add(dof_id_elem, dof_id_elem, -continuity_correction);
    }
  }

  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH ||
      _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    const auto dof_id_neighbor =
        _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];
    const auto sign = _current_face_type == FaceInfo::VarFaceNeighbors::BOTH ? 1.0 : -1.0;
    for (auto & matrix : _matrices)
      (*matrix).add(dof_id_neighbor, dof_id_neighbor, sign * continuity_correction);
  }
}

Real
LinearFVEnergyAdvection::computeElemMatrixContribution()
{
  return _cp * _advected_interp_coeffs.first * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeNeighborMatrixContribution()
{
  return _cp * _advected_interp_coeffs.second * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVEnergyAdvection::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVEnergyAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // Rhie-Chow mass fluxes on one-sided boundary faces are already oriented outward from the
  // active cell, even when the active cell is the FaceInfo neighbor.
  return _cp * boundary_value_matrix_contrib * _face_mass_flux * _current_face_area;
}

Real
LinearFVEnergyAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -_cp * boundary_value_rhs_contrib * _face_mass_flux * _current_face_area;
}

void
LinearFVEnergyAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the mass flux on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _face_mass_flux = _mass_flux_provider.getMassFlux(*face_info);

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms. For porous-medium cases, the Rhie-Chow object can override this hook while
  // energy transport still uses the superficial mass flux directly, i.e. without 1/epsilon
  // scaling of the advected quantity interpolation.
  _advected_interp_coeffs = _mass_flux_provider.getAdvectedInterpolationCoeffs(
      *_current_face_info, _advected_interp_method, _face_mass_flux, false);
}
