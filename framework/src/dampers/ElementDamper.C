//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementDamper.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
ElementDamper::validParams()
{
  InputParameters params = Damper::validParams();
  params += MaterialPropertyInterface::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this damper operates on");
  return params;
}

ElementDamper::ElementDamper(const InputParameters & parameters)
  : Damper(parameters),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, Moose::EMPTY_BOUNDARY_IDS),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, _sys.number())),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _current_elem(_var.currentElem()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),

    _u_increment(_var.increment()),

    _u(_var.sln()),
    _grad_u(_var.gradSln())
{
}

Real
ElementDamper::computeDamping()
{
  Real damping = 1.0;
  Real cur_damping = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    cur_damping = computeQpDamping();
    if (cur_damping < damping)
      damping = cur_damping;
  }

  return damping;
}
