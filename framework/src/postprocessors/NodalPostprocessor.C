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

#include "Moose.h"
#include "NodalPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<NodalPostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this postprocessor operates on");
  return params;
}

NodalPostprocessor::NodalPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    Coupleable(parameters, false),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    _block_id(parameters.get<unsigned int>("block")),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
//    _q_point(_subproblem.points(_tid)),
//    _qrule(_subproblem.qRule(_tid)),
//    _JxW(_subproblem.JxW(_tid)),
    _current_node(_var.node())
//    _u(_var.sln()),
//    _u_old(_var.slnOld()),
//    _u_older(_var.slnOlder()),
//    _grad_u(_var.gradSln()),
//    _grad_u_old(_var.gradSlnOld()),
//    _grad_u_older(_var.gradSlnOlder())
{
}

/*
Real
NodalPostprocessor::doSomething()
{

 *  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  return sum;

  return 0;
}
*/
