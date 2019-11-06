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

registerMooseObject("MooseTestApp", SplitTester);

InputParameters
SplitTester::validParams()
{
  return GeneralUserObject::validParams();
}

SplitTester::SplitTester(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
SplitTester::execute()
{
  if (!_fe_problem.mesh().isDistributedMesh())
    mooseError("Not using DistributedMesh but should be");
  auto & m = _fe_problem.mesh().getMesh();
  if (m.n_elem_on_proc(processor_id()) == m.n_elem())
    mooseError("Elements not shared properly between distributed mesh procs");
}
