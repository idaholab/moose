//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemElemConstraintBase.h"

// MOOSE includes
#include "Assembly.h"
#include "ElementPairInfo.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
ElemElemConstraintBase::validParams()
{
  InputParameters params = Constraint::validParams();
  params.addParam<unsigned int>("interface_id", 0, "The id of the interface.");
  return params;
}

ElemElemConstraintBase::ElemElemConstraintBase(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    NeighborMooseVariableInterface<Real>(
        this, false, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _dim(_mesh.dimension()),
    _interface_id(getParam<unsigned int>("interface_id")),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _current_elem(_assembly.elem()),

    _neighbor_elem(_assembly.neighbor()),

    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),

    _phi_neighbor(_assembly.phiNeighbor(_var)),
    _grad_phi_neighbor(_assembly.gradPhiNeighbor(_var)),

    _test_neighbor(_var.phiNeighbor()),
    _grad_test_neighbor(_var.gradPhiNeighbor()),

    _elem_residual_computed(false),
    _neighbor_residual_computed(false)
{
  addMooseVariableDependency(&_var);
}

void
ElemElemConstraintBase::reinit(const ElementPairInfo & element_pair_info)
{
  reinitConstraintQuadrature(element_pair_info);
}

void
ElemElemConstraintBase::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
{
  _constraint_q_point.resize(element_pair_info._elem1_constraint_q_point.size());
  _constraint_weight.resize(element_pair_info._elem1_constraint_JxW.size());
  std::copy(element_pair_info._elem1_constraint_q_point.begin(),
            element_pair_info._elem1_constraint_q_point.end(),
            _constraint_q_point.begin());
  std::copy(element_pair_info._elem1_constraint_JxW.begin(),
            element_pair_info._elem1_constraint_JxW.end(),
            _constraint_weight.begin());
}
