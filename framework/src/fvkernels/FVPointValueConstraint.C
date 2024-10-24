//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPointValueConstraint.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

registerMooseObject("MooseApp", FVPointValueConstraint);

InputParameters
FVPointValueConstraint::validParams()
{
  InputParameters params = FVScalarLagrangeMultiplierConstraint::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = volume * phi_0 "
                             "with a Lagrange multiplier approach.");
  params.setDocString("phi0", "What we want the point value of the primal variable to be.");
  params.addRequiredParam<Point>(
      "point", "The XYZ coordinates of the points where the value shall be enforced.");
  return params;
}

FVPointValueConstraint::FVPointValueConstraint(const InputParameters & parameters)
  : FVScalarLagrangeMultiplierConstraint(parameters),
    _point(getParam<Point>("point")),
    _my_elem(nullptr)
{
  setMyElem();
}

void
FVPointValueConstraint::setMyElem()
{
  // Find the element containing the point
  _point_locator = libMesh::PointLocatorBase::build(libMesh::TREE_LOCAL_ELEMENTS, _mesh);
  _point_locator->enable_out_of_mesh_mode();

  // We only check in the restricted blocks, if needed
  const Elem * elem =
      blockRestricted() ? (*_point_locator)(_point, &blockIDs()) : (*_point_locator)(_point);

  // We communicate the results and if there is conflict between processes,
  // the minimum cell ID is chosen
  const dof_id_type elem_id = elem ? elem->id() : libMesh::DofObject::invalid_id;
  dof_id_type min_elem_id = elem_id;
  _mesh.comm().min(min_elem_id);

  if (min_elem_id == libMesh::DofObject::invalid_id)
    mooseError("The specified point for the FVPointValueConstraint is not in the "
               "domain! Try alleviating block restrictions or "
               "using another point!");

  _my_elem = min_elem_id == elem_id ? elem : nullptr;
}

ADReal
FVPointValueConstraint::computeQpResidual()
{
  if (_current_elem == _my_elem)
    return _var(makeElemArg(_current_elem), determineState()) - _phi0;
  else
    return 0;
}
