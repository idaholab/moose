//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementUserObjectBasePD.h"
#include "AuxiliarySystem.h"
#include "PeridynamicsMesh.h"

InputParameters
ElementUserObjectBasePD::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Base class for peridynamic elemental user objects");

  return params;
}

ElementUserObjectBasePD::ElementUserObjectBasePD(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _aux(_fe_problem.getAuxiliarySystem()),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _nnodes(2)
{
}
