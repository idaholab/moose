//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralUserObjectBasePD.h"
#include "PeridynamicsMesh.h"

InputParameters
GeneralUserObjectBasePD::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for peridynamics general userobjects");

  return params;
}

GeneralUserObjectBasePD::GeneralUserObjectBasePD(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _aux(_fe_problem.getAuxiliarySystem()),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _nnodes(2)
{
}
