//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTurbulentTimeDerivative.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVTurbulentTimeDerivative);

InputParameters
LinearFVTurbulentTimeDerivative::validParams()
{
  InputParameters params = LinearFVTimeDerivative::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of a "
      "time derivative term for a turbulent variable in a partial differential equation.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  return params;
}

LinearFVTurbulentTimeDerivative::LinearFVTurbulentTimeDerivative(const InputParameters & params)
  : LinearFVTimeDerivative(params),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
}

void
LinearFVTurbulentTimeDerivative::initialSetup()
{
  LinearFVTimeDerivative::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

bool
LinearFVTurbulentTimeDerivative::currentElemIsWallBounded() const
{
  return _wall_bounded.find(_current_elem_info->elem()) != _wall_bounded.end();
}

void
LinearFVTurbulentTimeDerivative::addMatrixContribution()
{
  // The rows of wall bounded elements are set by the wall treatment
  if (!currentElemIsWallBounded())
    LinearFVTimeDerivative::addMatrixContribution();
}

void
LinearFVTurbulentTimeDerivative::addRightHandSideContribution()
{
  // The rows of wall bounded elements are set by the wall treatment
  if (!currentElemIsWallBounded())
    LinearFVTimeDerivative::addRightHandSideContribution();
}
