//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ErrorMaterial.h"
#include "NonlinearSystemBase.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", ErrorMaterial);

InputParameters
ErrorMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Test Material that throws MooseErrors at specified locations for testing purposes. We want "
      "to check that the materials are not computed in certain locations, as an optimization and a "
      "sanity check on the correct execution of the restriction logic");
  params.addParam<std::vector<BoundaryName>>(
      "sidesets_to_error_on",
      {},
      "List of sidesets to error at if the material is executed on that face");
  // Note: this could be extended to error on certain neighbor subdomains
  return params;
}

ErrorMaterial::ErrorMaterial(const InputParameters & parameters)
  : Material(parameters), _prop_value(declareProperty<Real>("matp"))
{
  // Get the boundary ids
  _boundary_ids_to_error = MooseMeshUtils::getBoundaryIDs(
      _mesh.getMesh(), getParam<std::vector<BoundaryName>>("sidesets_to_error_on"), false);
}

void
ErrorMaterial::computeQpProperties()
{
  // Just to check that got evaluated
  _prop_value[_qp] = 2.;

  // Check where we are, and error if needed
  // Note: neighbor materials have _bnd = true because they are using neighbor side materials
  // So this tests covers both face and neighbor (face) materials NOT executing
  if (_bnd)
  {
    std::vector<BoundaryID> boundary_ids = _mesh.getBoundaryIDs(_current_elem, _current_side);

    std::vector<BoundaryID> erroring_on_bid;

    std::sort(boundary_ids.begin(), boundary_ids.end());
    std::set_intersection(boundary_ids.begin(),
                          boundary_ids.end(),
                          _boundary_ids_to_error.begin(),
                          _boundary_ids_to_error.end(),
                          back_inserter(erroring_on_bid));
    if (erroring_on_bid.size())
      mooseError("Erroring on boundary '" + _mesh.getBoundaryName(erroring_on_bid[0]) +
                 "' as requested");
  }
}
