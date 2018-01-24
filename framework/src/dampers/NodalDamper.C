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
#include "MooseVariable.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<NodalDamper>()
{
  InputParameters params = validParams<Damper>();
  params += validParams<MaterialPropertyInterface>();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this damper operates on");
  return params;
}

NodalDamper::NodalDamper(const InputParameters & parameters)
  : Damper(parameters),
    MaterialPropertyInterface(this),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _current_node(_var.node()),
    _qp(0),
    _u_increment(_var.increment()),
    _u(_var.nodalSln())
{
}

Real
NodalDamper::computeDamping()
{
  return computeQpDamping();
}
