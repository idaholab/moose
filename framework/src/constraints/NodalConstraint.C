//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/sparse_matrix.h"

InputParameters
NodalConstraint::validParams()
{
  InputParameters params = Constraint::validParams();
  MooseEnum formulationtype("penalty kinematic", "penalty");
  params.addParam<MooseEnum>("formulation",
                             formulationtype,
                             "Formulation used to calculate constraint - penalty or kinematic.");
  params.addParam<NonlinearVariableName>("variable_secondary",
                                         "The name of the variable for the secondary nodes, if it "
                                         "is different from the primary nodes' variable");
  return params;
}

NodalConstraint::NodalConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, true),
    NeighborMooseVariableInterface<Real>(
        this, true, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(_sys.getFieldVariable<Real>(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _var_secondary(_sys.getFieldVariable<Real>(
        _tid,
        isParamValid("variable_secondary")
            ? parameters.get<NonlinearVariableName>("variable_secondary")
            : parameters.get<NonlinearVariableName>("variable"))),
    _u_secondary(_var_secondary.dofValuesNeighbor()),
    _u_primary(_var.dofValues())
{
  addMooseVariableDependency(&_var);
  addMooseVariableDependency(&_var_secondary);

  MooseEnum temp_formulation = getParam<MooseEnum>("formulation");
  if (temp_formulation == "penalty")
    _formulation = Moose::Penalty;
  else if (temp_formulation == "kinematic")
    _formulation = Moose::Kinematic;
  else
    mooseError("Formulation must be either Penalty or Kinematic");
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  if ((_weights.size() == 0) && (_primary_node_vector.size() == 1))
    _weights.push_back(1.0);

  std::vector<dof_id_type> primarydof = _var.dofIndices();
  std::vector<dof_id_type> secondarydof = _var_secondary.dofIndicesNeighbor();

  DenseVector<Number> re(primarydof.size());
  DenseVector<Number> neighbor_re(secondarydof.size());

  re.zero();
  neighbor_re.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    for (_j = 0; _j < primarydof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Penalty:
          re(_j) += computeQpResidual(Moose::Primary) * _var.scalingFactor();
          neighbor_re(_i) += computeQpResidual(Moose::Secondary) * _var_secondary.scalingFactor();
          break;
        case Moose::Kinematic:
          // Transfer the current residual of the secondary node to the primary nodes
          Real res = residual(secondarydof[_i]);
          re(_j) += res * _weights[_j];
          neighbor_re(_i) +=
              -res / _primary_node_vector.size() + computeQpResidual(Moose::Secondary);
          break;
      }
    }
  }
  // We've already applied scaling
  if (!primarydof.empty())
    addResiduals(_assembly, re, primarydof, /*scaling_factor=*/1);
  if (!secondarydof.empty())
    addResiduals(_assembly, neighbor_re, secondarydof, /*scaling_factor=*/1);
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  if ((_weights.size() == 0) && (_primary_node_vector.size() == 1))
    _weights.push_back(1.0);

  // Calculate the dense-block Jacobian entries
  std::vector<dof_id_type> secondarydof = _var_secondary.dofIndicesNeighbor();
  std::vector<dof_id_type> primarydof = _var.dofIndices();

  DenseMatrix<Number> Kee(primarydof.size(), primarydof.size());
  DenseMatrix<Number> Ken(primarydof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), primarydof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    for (_j = 0; _j < primarydof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Penalty:
          Kee(_j, _j) += computeQpJacobian(Moose::PrimaryPrimary);
          Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
          Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
          break;
        case Moose::Kinematic:
          Kee(_j, _j) = 0.;
          Ken(_j, _i) += jacobian(secondarydof[_i], primarydof[_j]) * _weights[_j];
          Kne(_i, _j) += -jacobian(secondarydof[_i], primarydof[_j]) / primarydof.size() +
                         computeQpJacobian(Moose::SecondaryPrimary);
          break;
      }
    }
  }
  addJacobian(_assembly, Kee, primarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var_secondary.scalingFactor());

  // Calculate and cache the diagonal secondary-secondary entries
  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    Number value = 0.0;
    switch (_formulation)
    {
      case Moose::Penalty:
        value = computeQpJacobian(Moose::SecondarySecondary);
        break;
      case Moose::Kinematic:
        value = -jacobian(secondarydof[_i], secondarydof[_i]) / primarydof.size() +
                computeQpJacobian(Moose::SecondarySecondary);
        break;
    }
    addJacobianElement(
        _assembly, value, secondarydof[_i], secondarydof[_i], _var_secondary.scalingFactor());
  }
}

void
NodalConstraint::updateConnectivity()
{
}
