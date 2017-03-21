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

#include "NodalDamper.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "FEProblem.h"

// libMesh includes
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
