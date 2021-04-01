//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeFaceConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

#include <unordered_map>

// Forward Declarations
class FEProblem;

class ApplyPenetrationConstraintLMMechanicalContact : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  ApplyPenetrationConstraintLMMechanicalContact(const InputParameters & parameters);

  bool shouldApply() override;
  bool overwriteSecondaryResidual() override;
  void residualSetup() override;

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  /// A scaling factor meant to put the weighted gap and the contact pressure on the same scale
  const Real _c;

  /// a copy of the ghosted residual vector such that we can read the weighted gap
  const NumericVector<Number> & _residual_copy;

  /// The dof number for the LM variable at the current node
  dof_id_type _dof_number;

  /// Whether we successfully projected the secondary node onto the primary face
  bool _projection_successful;

  /// A map from node to integrated gap. This is useful for Jacobian evaluations when our residual
  /// copy is not what we want it to be (e.g. the residual entries have potentially been overwritten
  /// by the lagrange multiplier)
  std::unordered_map<const Node *, Real> _node_to_integrated_gap;
};
