//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTurbulentLimitedAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVTurbulentLimitedAdvection);

InputParameters
LinearFVTurbulentLimitedAdvection::validParams()
{
  InputParameters params = LinearFVScalarAdvection::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for a turbulent variable.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");

  return params;
}

LinearFVTurbulentLimitedAdvection::LinearFVTurbulentLimitedAdvection(const InputParameters & params)
  : LinearFVScalarAdvection(params),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_mass_flux(0.0),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

void
LinearFVTurbulentLimitedAdvection::initialSetup()
{
  LinearFVScalarAdvection::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

Real
LinearFVTurbulentLimitedAdvection::computeElemMatrixContribution()
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  return bounded_elem ? _advected_interp_coeffs.first * _face_mass_flux * _current_face_area : 0.0;
}

Real
LinearFVTurbulentLimitedAdvection::computeNeighborMatrixContribution()
{
  const Elem * neighbor = _current_face_info->neighborPtr();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();
  return bounded_neigh ? _advected_interp_coeffs.second * _face_mass_flux * _current_face_area : 0.0;
}

Real
LinearFVTurbulentLimitedAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();

  if (!bounded_elem)
  {
    const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
    mooseAssert(adv_bc, "This should be a valid BC!");

    const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

    // We support internal boundaries too so we have to make sure the normal points always outward
    const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

    return boundary_value_matrix_contrib * factor * _face_mass_flux * _current_face_area;
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();

  if (!bounded_elem)
  {
    const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
    mooseAssert(adv_bc, "This should be a valid BC!");

    // We support internal boundaries too so we have to make sure the normal points always outward
    const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

    const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
    return -boundary_value_rhs_contrib * factor * _face_mass_flux * _current_face_area;
  }
  else
    return 0.0;
}