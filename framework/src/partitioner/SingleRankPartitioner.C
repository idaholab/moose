//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingleRankPartitioner.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", SingleRankPartitioner);

InputParameters
SingleRankPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();

  params.addParam<processor_id_type>("rank", 0, "The MPI rank to assign all elements to.");

  params.addClassDescription("Assigns element processor ids to a single MPI rank.");

  return params;
}

SingleRankPartitioner::SingleRankPartitioner(const InputParameters & params)
  : MoosePartitioner(params), _rank(getParam<processor_id_type>("rank"))
{
  if (_rank >= _communicator.size())
    paramError("rank", "Cannot be larger than the available number of MPI ranks");
}

std::unique_ptr<Partitioner>
SingleRankPartitioner::clone() const
{
  return std::make_unique<SingleRankPartitioner>(_pars);
}

void
SingleRankPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  for (auto & elem_ptr : mesh.active_element_ptr_range())
    elem_ptr->processor_id() = _rank;
}
