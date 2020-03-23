//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundsAuxBase.h"
#include "SystemBase.h"
#include "PetscSupport.h"

InputParameters
BoundsAuxBase::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum type_options("upper=0 lower=1", "upper");
  params.addParam<MooseEnum>(
      "bound_type",
      type_options,
      "Type of bound. 'upper' refers to the upper bound. 'lower' refers to the lower value.");
  params.addRequiredParam<NonlinearVariableName>("bounded_variable", "The variable to be bounded");
  return params;
}

BoundsAuxBase::BoundsAuxBase(const InputParameters & parameters)
  : AuxKernel(parameters),
    _type((BoundType)(int)parameters.get<MooseEnum>("bound_type")),
    _bounded_vector(_type == 0 ? _nl_sys.getVector("upper_bound")
                               : _nl_sys.getVector("lower_bound")),
    _bounded_var(_nl_sys.getVariable(_tid, getParam<NonlinearVariableName>("bounded_variable"))),
    _bounded_var_name(parameters.get<NonlinearVariableName>("bounded_variable")),
    _var(_subproblem.getStandardVariable(_tid, _bounded_var_name))
{
  if (!isNodal())
    mooseError("BoundsAuxBase must be used on a nodal auxiliary variable!");
  if (!Moose::PetscSupport::isSNESVI(*dynamic_cast<FEProblemBase *>(&_subproblem)))
    mooseDoOnce(mooseWarning(
        "A variational inequalities solver must be used in conjunction with BoundsAux"));
}

Real
BoundsAuxBase::computeValue()
{
  if (_current_node->n_dofs(_nl_sys.number(), _bounded_var.number()) > 0)
  {
    // The zero is for the component, this will only work for Lagrange variables!
    dof_id_type dof = _current_node->dof_number(_nl_sys.number(), _bounded_var.number(), 0);

    Real bound = getBound();
    _bounded_vector.set(dof, bound);
  }

  return 0.0;
}
