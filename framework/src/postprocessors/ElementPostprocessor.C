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
#include "ElementPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this postprocessor operates on");
  std::vector<std::string> everywhere(1);
  everywhere[0] = "ANY_BLOCK_ID";
  params.addParam<std::vector<SubdomainName> >("block", everywhere, "block ID or name where the postprocessor works");
  return params;
}

ElementPostprocessor::ElementPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    Coupleable(parameters, false),
    UserObjectInterface(parameters),
    MooseVariableInterface(parameters, false),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    _blocks(parameters.get<std::vector<std::string> >("block")),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),
    _current_elem(_subproblem.elem(_tid)),
    _current_elem_volume(_subproblem.elemVolume(_tid)),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    //
    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}
