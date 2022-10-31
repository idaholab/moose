//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalDamper.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
NodalDamper::validParams()
{
  InputParameters params = Damper::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this damper operates on");
  return params;
}

NodalDamper::NodalDamper(const InputParameters & parameters)
  : Damper(parameters),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, _sys.number())),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _current_node(_var.node()),
    _qp(0),
    _u_increment(_var.increment()),
    _u(_var.dofValues())
{
}

Real
NodalDamper::computeDamping()
{
  return computeQpDamping();
}
