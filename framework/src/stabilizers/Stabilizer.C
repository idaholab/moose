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

#include "Stabilizer.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<Stabilizer>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");

  params.addPrivateParam<std::string>("built_by_action", "add_stabilizer");
  return params;
}


Stabilizer::Stabilizer(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  MaterialPropertyInterface(parameters),
  _subproblem(*parameters.get<SubProblem *>("_subproblem")),
  _tid(parameters.get<THREAD_ID>("_tid")),

  _var(_subproblem.getVariable(_tid, parameters.get<std::string>("variable"))),
  _current_elem(_var.currentElem()),

  _q_point(_subproblem.points(_tid)),
  _qrule(_subproblem.qRule(_tid)),
  _JxW(_subproblem.JxW(_tid)),

  _phi(_var.phi()),
  _grad_phi(_var.gradPhi()),
  _test(_var.test()),
  _grad_test(_var.gradTest())
{
}

Stabilizer::~Stabilizer()
{
}
