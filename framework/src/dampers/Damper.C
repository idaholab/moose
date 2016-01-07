/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Damper.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<Damper>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<MaterialPropertyInterface>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this damper operates on");
  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("Damper");
  return params;
}

Damper::Damper(const InputParameters & parameters) :
    MooseObject(parameters),
    SetupInterface(parameters),
    MaterialPropertyInterface(parameters),
    Restartable(parameters, "Dampers"),
    MeshChangedInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _coord_sys(_assembly.coordSystem()),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),

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
Damper::computeDamping()
{
  Real damping = 1.0;
  Real cur_damping = 1.0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    cur_damping = computeQpDamping();
    if (cur_damping < damping)
      damping = cur_damping;
  }

  return damping;
}
