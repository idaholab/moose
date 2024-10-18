//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundsBase.h"
#include "SystemBase.h"
#include "PetscSupport.h"
#include "AuxiliarySystem.h"

InputParameters
BoundsBase::validParams()
{
  InputParameters params = AuxKernel::validParams();
  MooseEnum type_options("upper=0 lower=1", "upper");
  params.addParam<MooseEnum>(
      "bound_type",
      type_options,
      "Type of bound. 'upper' refers to the upper bound. 'lower' refers to the lower value.");
  params.addRequiredParam<NonlinearVariableName>("bounded_variable", "The variable to be bounded");
  params.registerBase("Bounds");
  return params;
}

BoundsBase::BoundsBase(const InputParameters & parameters)
  : AuxKernel(parameters),
    _type((BoundType)(int)parameters.get<MooseEnum>("bound_type")),
    _bounded_vector(_type == 0 ? _nl_sys.getVector("upper_bound")
                               : _nl_sys.getVector("lower_bound")),
    _bounded_var(_nl_sys.getVariable(_tid, getParam<NonlinearVariableName>("bounded_variable"))),
    _bounded_var_name(parameters.get<NonlinearVariableName>("bounded_variable")),
    _fe_var(_bounded_var.isFV() ? nullptr
                                : &_subproblem.getStandardVariable(_tid, _bounded_var_name))
{
  if (!Moose::PetscSupport::isSNESVI(*dynamic_cast<FEProblemBase *>(&_subproblem)))
    mooseDoOnce(
        mooseWarning("A variational inequalities solver must be used in conjunction with Bounds"));

  // Check that the bounded variable is of a supported type
  if (!_bounded_var.isNodal() && (_bounded_var.feType().order != libMesh::CONSTANT))
    paramError("bounded_variable", "Bounded variable must be nodal or of a CONSTANT order!");

  const auto & dummy =
      _aux_sys.getActualFieldVariable<Real>(_tid, parameters.get<AuxVariableName>("variable"));

  // Check that the dummy variable matches the bounded variable
  if (dummy.feType() != _bounded_var.feType())
    paramError("variable",
               "Dummy bounds aux variable and bounded variable must use the same finite element "
               "order and family");
}

Real
BoundsBase::computeValue()
{
  dof_id_type dof = getDoFIndex();

  if (dof != std::numeric_limits<dof_id_type>::max())
  {
    Real bound = getBound();
    _bounded_vector.set(dof, bound);
  }

  return 0.0;
}

dof_id_type
BoundsBase::getDoFIndex() const
{
  if (isNodal())
  {
    if (_current_node->n_dofs(_nl_sys.number(), _bounded_var.number()) > 0)
    {
      mooseAssert(_current_node->n_dofs(_nl_sys.number(), _bounded_var.number()) == 1,
                  "Bounds are only set on one DOF value per node currently");
      // The zero is for the component, this will only work for Lagrange variables
      return _current_node->dof_number(_nl_sys.number(), _bounded_var.number(), 0);
    }
  }
  else if (_current_elem->n_dofs(_nl_sys.number(), _bounded_var.number()) > 0)
  {
    mooseAssert(_current_elem->n_dofs(_nl_sys.number(), _bounded_var.number()) == 1,
                "Bounds are only set on one DOF value per element currently");
    // The zero is for the component, this will only work for CONSTANT variables
    return _current_elem->dof_number(_nl_sys.number(), _bounded_var.number(), 0);
  }
  // No local dof for the bounded variable. This can happen for example if:
  // - block restriction of dummy is different from bounded variable
  // - we have a first order variable on a second order mesh
  return std::numeric_limits<dof_id_type>::max();
}
