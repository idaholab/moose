//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemElemConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "ElementPairInfo.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
ElemElemConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  params.addParam<unsigned int>("interface_id", 0, "The id of the interface.");
  return params;
}

ElemElemConstraint::ElemElemConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    NeighborMooseVariableInterface<Real>(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _dim(_mesh.dimension()),
    _interface_id(getParam<unsigned int>("interface_id")),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),

    _current_elem(_assembly.elem()),

    _neighbor_elem(_assembly.neighbor()),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),

    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),

    _phi_neighbor(_assembly.phiNeighbor(_var)),
    _grad_phi_neighbor(_assembly.gradPhiNeighbor(_var)),

    _test_neighbor(_var.phiNeighbor()),
    _grad_test_neighbor(_var.gradPhiNeighbor()),

    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
  addMooseVariableDependency(&_var);
}

void
ElemElemConstraint::reinit(const ElementPairInfo & element_pair_info)
{
  reinitConstraintQuadrature(element_pair_info);
}

void
ElemElemConstraint::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
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

void
ElemElemConstraint::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _var.number());
  for (_qp = 0; _qp < _constraint_q_point.size(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _constraint_weight[_qp] * computeQpResidual(type);
  accumulateTaggedLocalResidual();
}

void
ElemElemConstraint::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
ElemElemConstraint::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), type);

  for (_qp = 0; _qp < _constraint_q_point.size(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _constraint_weight[_qp] * computeQpJacobian(type);

  accumulateTaggedLocalMatrix();
}

void
ElemElemConstraint::computeJacobian()
{
  // Compute element-element Jacobian
  computeElemNeighJacobian(Moose::ElementElement);

  // Compute element-neighbor Jacobian
  computeElemNeighJacobian(Moose::ElementNeighbor);

  // Compute neighbor-element Jacobian
  computeElemNeighJacobian(Moose::NeighborElement);

  // Compute neighbor-neighbor Jacobian
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}
