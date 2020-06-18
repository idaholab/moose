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
#include "MooseEnum.h"

#include <vector>
#include <unordered_map>

class PenetrationInfo;

namespace libMesh
{
template <typename>
class NumericVector;
}

class RANFSTieNode : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  RANFSTieNode(const InputParameters & parameters);

  bool shouldApply() override;
  void residualSetup() override;
  bool overwriteSecondaryResidual() override;
  void computeSecondaryValue(NumericVector<Number> & solution) override;

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  const MooseEnum _component;
  const unsigned int _mesh_dimension;
  NumericVector<Number> & _residual_copy;

  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;
  Real _lagrange_multiplier;
  const Node * _nearest_node;
  std::unordered_map<dof_id_type, Real> _node_to_lm;
  dof_id_type _primary_index;

  std::unordered_map<dof_id_type, Number> _dof_number_to_value;
};
