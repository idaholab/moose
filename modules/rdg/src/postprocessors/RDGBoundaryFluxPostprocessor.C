//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RDGBoundaryFluxPostprocessor.h"
#include "BoundaryFluxBase.h"

registerMooseObject("RdgApp", RDGBoundaryFluxPostprocessor);
registerMooseObjectRenamed("RdgApp",
                           BoundaryFluxPostprocessor,
                           "04/01/2025 00:00",
                           RDGBoundaryFluxPostprocessor);

InputParameters
RDGBoundaryFluxPostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();

  params.addRequiredParam<UserObjectName>("boundary_flux_uo", "Boundary flux user object name");
  params.addRequiredParam<unsigned int>("flux_index", "Index within flux vector to query");
  params.addParam<Point>("normal", "Normal vector for boundary (if requesting override)");
  params.addRequiredCoupledVar(
      "variables", "Variables to pass to boundary flux user object, in the correct order");

  params.addClassDescription(
      "Computes the side integral of a flux entry from a BoundaryFluxBase user object");

  return params;
}

RDGBoundaryFluxPostprocessor::RDGBoundaryFluxPostprocessor(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),

    _boundary_flux_uo(getUserObject<BoundaryFluxBase>("boundary_flux_uo")),
    _flux_index(getParam<unsigned int>("flux_index")),
    _provided_normal(isParamValid("normal")),
    _n_components(coupledComponents("variables")),
    _U(coupledValues("variables"))
{
}

Real
RDGBoundaryFluxPostprocessor::computeQpIntegral()
{
  std::vector<Real> U(_n_components);
  for (unsigned int i = 0; i < _n_components; i++)
    U[i] = (*_U[i])[_qp];

  const Point & normal = _provided_normal ? getParam<Point>("normal") : _normals[_qp];

  const auto & flux = _boundary_flux_uo.getFlux(_current_side, _current_elem->id(), U, normal);
  return flux[_flux_index];
}
