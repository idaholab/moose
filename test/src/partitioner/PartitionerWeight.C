//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PartitionerWeight.h"

registerMooseObject("MooseApp", PartitionerWeight);

template <>
InputParameters
validParams<PartitionerWeight>()
{
  InputParameters params = validParams<PetscMatPartitioner>();

  params.addClassDescription("Partition mesh using the weighted graph");

  return params;
}

PartitionerWeight::PartitionerWeight(const InputParameters & params) : PetscMatPartitioner(params)
{
}

std::unique_ptr<Partitioner>
PartitionerWeight::clone() const
{
  return libmesh_make_unique<PartitionerWeight>(_pars);
}

dof_id_type
PartitionerWeight::computeElementWeight(Elem & elem)
{
  auto centroid = elem.centroid();
  if (centroid(0) < 0.5)
    return 2;
  else
    return 1;
}

dof_id_type
PartitionerWeight::computeSideWeight(Elem & /*elem*/, Elem & side)
{
  auto centroid = side.centroid();
  if (centroid(0) == 0.5)
    return 20;
  else
    return 1;
}
