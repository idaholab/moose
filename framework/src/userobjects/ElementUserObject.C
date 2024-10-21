//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementUserObject.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/elem.h"

InputParameters
ElementUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += BlockRestrictable::validParams();
  params += MaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params += RandomInterface::validParams();
  return params;
}

ElementUserObject::ElementUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    BlockRestrictable(this),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    TransientInterface(this),
    RandomInterface(parameters, _fe_problem, _tid, false),
    ElementIDInterface(this),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
}
