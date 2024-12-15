//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTurbulentLimitedDiffusion.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("MooseApp", LinearFVTurbulentLimitedDiffusion);

InputParameters
LinearFVTurbulentLimitedDiffusion::validParams()
{
  InputParameters params = LinearFVDiffusion::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of a "
                             "diffusion term for a turbulent variable in a partial differential equation.");

  params.addParam<MooseFunctorName>("scaling_coeff", 1.0, "The scaling coefficient for the diffusion term.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  return params;
}

LinearFVTurbulentLimitedDiffusion::LinearFVTurbulentLimitedDiffusion(const InputParameters & params)
  : LinearFVDiffusion(params),
    _scaling_coeff(getFunctor<Real>("scaling_coeff")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  if (_use_nonorthogonal_correction)
    _var.computeCellGradients();
}

void
LinearFVTurbulentLimitedDiffusion::initialSetup()
{
  LinearFVDiffusion::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

Real
LinearFVTurbulentLimitedDiffusion::computeElemMatrixContribution()
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();

  if (!bounded_elem)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    return computeFluxMatrixContribution() / _scaling_coeff(face_arg, determineState());
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedDiffusion::computeNeighborMatrixContribution()
{
  const Elem * neighbor = _current_face_info->neighborPtr();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  if (!bounded_neigh)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    return -computeFluxMatrixContribution() / _scaling_coeff(face_arg, determineState());
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedDiffusion::computeElemRightHandSideContribution()
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  
  if (!bounded_elem)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    return computeFluxRHSContribution() / _scaling_coeff(face_arg, determineState());
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedDiffusion::computeNeighborRightHandSideContribution()
{
  const Elem * neighbor = _current_face_info->neighborPtr();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  if (!bounded_neigh)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    return -computeFluxRHSContribution() / _scaling_coeff(face_arg, determineState());
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedDiffusion::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();

  if (!bounded_elem)
  {
    const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
    mooseAssert(diff_bc, "This should be a valid BC!");

    auto grad_contrib = diff_bc->computeBoundaryGradientMatrixContribution() * _current_face_area;
    // If the boundary condition does not include the diffusivity contribution then
    // add it here.
    if (!diff_bc->includesMaterialPropertyMultiplier())
    {
        const auto face_arg = singleSidedFaceArg(_current_face_info);
        grad_contrib *= _diffusion_coeff(face_arg, determineState());
    }

    return grad_contrib;
  }
  else
    return 0.0;
}

Real
LinearFVTurbulentLimitedDiffusion::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();

  if (!bounded_elem)
  {
    const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
    mooseAssert(diff_bc, "This should be a valid BC!");

    const auto face_arg = singleSidedFaceArg(_current_face_info);
    auto grad_contrib = diff_bc->computeBoundaryGradientRHSContribution() * _current_face_area;

    // If the boundary condition does not include the diffusivity contribution then
    // add it here.
    if (!diff_bc->includesMaterialPropertyMultiplier())
        grad_contrib *= _diffusion_coeff(face_arg, determineState());

    // We add the nonorthogonal corrector for the face here. Potential idea: we could do
    // this in the boundary condition too. For now, however, we keep it like this.
    if (_use_nonorthogonal_correction)
    {
        const auto correction_vector =
            _current_face_info->normal() -
            1 / (_current_face_info->normal() * _current_face_info->eCN()) * _current_face_info->eCN();

        grad_contrib += _diffusion_coeff(face_arg, determineState()) *
                        _var.gradSln(*_current_face_info->elemInfo()) * correction_vector *
                        _current_face_area;
    }

    return grad_contrib;
  }
  else
    return 0.0;
}