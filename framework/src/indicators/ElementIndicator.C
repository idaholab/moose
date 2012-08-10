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
  params.addParam<std::vector<SubdomainName> >("block", "The list of ids of the blocks (subdomain) that this Indicator will be applied to");
  params += validParams<TransientInterface>();
  return params;
}


ElementIndicator::ElementIndicator(const std::string & name, InputParameters parameters) :
    Indicator(name, parameters),
    TransientInterface(parameters, name, "indicators"),
    PostprocessorInterface(parameters),

    _current_elem(_field_var.currentElem()),
    _current_elem_volume(_subproblem.elemVolume(_tid)),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),

    _u(_is_implicit ? _field_var.sln() : _field_var.slnOld()),
    _grad_u(_is_implicit ? _field_var.gradSln() : _field_var.gradSlnOld()),
    _u_dot(_field_var.uDot()),
    _du_dot_du(_field_var.duDotDu()),

    _real_zero(_subproblem._real_zero[_tid]),
    _zero(_subproblem._zero[_tid]),
    _grad_zero(_subproblem._grad_zero[_tid]),
    _second_zero(_subproblem._second_zero[_tid])
{
}
