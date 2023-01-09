//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterMesh.h"

#include "libmesh/enum_point_locator_type.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/fe_compute_data.h"
#include "libmesh/fe_interface.h"

ParameterMesh::ParameterMesh(const FEType & param_type, const std::string & exodus_mesh)
  : _communicator(MPI_COMM_SELF), _mesh(_communicator)
{
  _mesh.allow_renumbering(false);
  _mesh.read(exodus_mesh);

  // Create system to store parameter values
  _eq = std::make_unique<EquationSystems>(_mesh);
  _sys = &_eq->add_system<ExplicitSystem>("_parameter_mesh_sys");
  _sys->add_variable("_parameter_mesh_var", param_type);
  _eq->init();

  // Create point locator
  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh);
  _point_locator->enable_out_of_mesh_mode();
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<Real> & weights) const
{
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(pt);
  if (!elem)
    mooseError("No element was found to contain point ", pt);

  // Get the  in the dof_indices for our element
  const unsigned int var = 0;
  const DofMap & dof_map = _sys->get_dof_map();
  dof_map.dof_indices(elem, dof_indices, var);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, pt);
  // get the shape function value via the FEInterface
  FEType fe_type = dof_map.variable_type(var);
  FEComputeData fe_data(*_eq, coor);
  FEInterface::compute_data(elem->dim(), fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.shape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}

void
ParameterMesh::getIndexAndWeight(const Point & pt,
                                 std::vector<dof_id_type> & dof_indices,
                                 std::vector<RealGradient> & weights) const
{
  // Locate the element the point is in
  const Elem * elem = (*_point_locator)(pt);

  // Get the  in the dof_indices for our element
  const unsigned int var = 0;
  const DofMap & dof_map = _sys->get_dof_map();
  dof_map.dof_indices(elem, dof_indices, var);

  // Map the physical co-ordinates to the reference co-ordinates
  Point coor = FEMap::inverse_map(elem->dim(), elem, pt);
  // get the shape function value via the FEInterface
  FEType fe_type = dof_map.variable_type(var);
  FEComputeData fe_data(*_eq, coor);
  fe_data.enable_derivative();
  FEInterface::compute_data(elem->dim(), fe_type, elem, fe_data);
  // Set weights to the value of the shape functions
  weights = fe_data.dshape;

  if (dof_indices.size() != weights.size())
    mooseError("Internal error: weights and DoF indices do not have the same size.");
}
