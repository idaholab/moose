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

#include "ElementIndicator.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "threads.h"

template<>
InputParameters validParams<ElementIndicator>()
{
  InputParameters params = validParams<Indicator>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this Indicator operates on");

  std::vector<SubdomainName> everywhere(1);
  everywhere[0] = "ANY_BLOCK_ID";
  params.addParam<std::vector<SubdomainName> >("block", everywhere, "block ID or name where the object works");

  params += validParams<TransientInterface>();
  return params;
}


ElementIndicator::ElementIndicator(const std::string & name, InputParameters parameters) :
    Indicator(name, parameters),
    TransientInterface(parameters, name, "indicators"),
    PostprocessorInterface(parameters),
    Coupleable(parameters, false),
    ScalarCoupleable(parameters),
    MooseVariableInterface(parameters, false),
    MaterialPropertyInterface(parameters),

    _field_var(_sys.getVariable(_tid, name)),

    _current_elem(_field_var.currentElem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);

  addMooseVariableDependency(mooseVariable());
}
