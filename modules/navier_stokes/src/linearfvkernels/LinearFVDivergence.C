//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDivergence.h"

registerMooseObject("NavierStokesApp", LinearFVDivergence);

InputParameters
LinearFVDivergence::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents a divergence term. Note, this term does not contribute to "
                             "the system matrix, only takes the divergence of a face flux field "
                             "and adds it to the right hand side of the linear system.");
  params.addRequiredParam<MooseFunctorName>("face_flux", "Functor for the face flux.");
  return params;
}

LinearFVDivergence::LinearFVDivergence(const InputParameters & params)
  : LinearFVFluxKernel(params), _face_flux(getFunctor<Real>("face_flux"))

{
}

Real
LinearFVDivergence::computeElemMatrixContribution()
{
  return 0;
}

Real
LinearFVDivergence::computeNeighborMatrixContribution()
{
  return 0;
}

Real
LinearFVDivergence::computeElemRightHandSideContribution()
{
  return computeFaceFlux();
}

Real
LinearFVDivergence::computeNeighborRightHandSideContribution()
{
  return -computeFaceFlux();
}

Real
LinearFVDivergence::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return 0.0;
}

Real
LinearFVDivergence::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & /*bc*/)
{
  return computeFaceFlux();
}

Real
LinearFVDivergence::computeFaceFlux()
{
  const auto face_arg = makeCDFace(*_current_face_info);
  const auto state_arg = determineState();

  if (!_cached_rhs_contribution)
  {
    _cached_rhs_contribution = true;
    _flux_rhs_contribution = _face_flux(face_arg, state_arg) * _current_face_area;
  }

  return _flux_rhs_contribution;
}
