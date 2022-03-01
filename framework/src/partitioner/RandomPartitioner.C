//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomPartitioner.h"

#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseRandom.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", RandomPartitioner);

InputParameters
RandomPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();

  params.addParam<unsigned int>("seed", 0, "Seed for the random generator");

  params.addClassDescription("Assigns element processor ids randomly with a given seed.");

  return params;
}

RandomPartitioner::RandomPartitioner(const InputParameters & params)
  : MoosePartitioner(params), _num_procs(_app.getCommunicator()->size())
{
  MooseRandom::seed(getParam<unsigned int>("seed"));
}

RandomPartitioner::~RandomPartitioner() {}

std::unique_ptr<Partitioner>
RandomPartitioner::clone() const
{
  return std::make_unique<RandomPartitioner>(_pars);
}

void
RandomPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  // Random number is on [0, 1]: scale to number of procs and round down
  for (auto & elem_ptr : mesh.active_element_ptr_range())
    elem_ptr->processor_id() = std::floor(MooseRandom::rand() * _num_procs);
}
