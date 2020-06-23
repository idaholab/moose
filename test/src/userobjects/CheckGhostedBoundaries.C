//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckGhostedBoundaries.h"

registerMooseObject("MooseTestApp", CheckGhostedBoundaries);

InputParameters
CheckGhostedBoundaries::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<dof_id_type>("total_num_bdry_sides", "Total number of boundary sides");
  return params;
}

CheckGhostedBoundaries::CheckGhostedBoundaries(const InputParameters & params)
  : GeneralUserObject(params), _total_num_bdry_sides(getParam<dof_id_type>("total_num_bdry_sides"))
{
}

void
CheckGhostedBoundaries::execute()
{
  dof_id_type nelems = _fe_problem.mesh().getMesh().get_boundary_info().build_side_list().size();

  // If the total number of boundary sides is the same as the number of local boundary sides,
  // we will conclude all-gather happens. It is not necessarily true, but it works fine
  // for setting up tests
  if (_total_num_bdry_sides == nelems)
    mooseError("All boundaries are ghosted to every single processor, and it is not scalable.");
}
