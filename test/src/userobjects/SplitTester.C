//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitTester.h"
#include "MooseMesh.h"

#include "libmesh/distributed_mesh.h"

template <>
InputParameters
validParams<SplitTester>()
{
  return validParams<GeneralUserObject>();
  // pars.set<MultiMooseEnum>("execute_on") = "timestep_begin";
}

SplitTester::SplitTester(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
SplitTester::execute()
{
  auto & m = _fe_problem.mesh().getMesh();
  if (!dynamic_cast<DistributedMesh *>(&m))
    mooseError("not using DistributedMesh but should be");
  if (m.n_elem_on_proc(processor_id()) == m.n_elem())
    mooseError("elements not shared properly between distributed mesh procs");
}
