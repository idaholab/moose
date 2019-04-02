//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DoFPartitioner.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "NonlinearSystemBase.h"

// libmesh include
#include "libmesh/elem.h"

registerMooseObject("MooseApp", DoFPartitioner);

template <>
InputParameters
validParams<DoFPartitioner>()
{
  InputParameters params = validParams<PetscExternalPartitioner>();
  params.set<bool>("apply_element_weight") = true;
  params.suppressParameter<bool>("apply_element_weight");
  params.addClassDescription(
      "Partitions the mesh using the number of dofs on each element as weight.");
  return params;
}

DoFPartitioner::DoFPartitioner(const InputParameters & params) : PetscExternalPartitioner(params) {}

std::unique_ptr<Partitioner>
DoFPartitioner::clone() const
{
  return libmesh_make_unique<DoFPartitioner>(_pars);
}

dof_id_type
DoFPartitioner::computeElementWeight(Elem & elm)
{
  // until changes to mesh reading & partitioning order
  // are made, the next lines returns 0s
  return _app.blockDoFs(elm.subdomain_id());
}
