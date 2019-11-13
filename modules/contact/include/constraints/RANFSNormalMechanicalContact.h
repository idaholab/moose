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

class RANFSNormalMechanicalContact;
class PenetrationInfo;

namespace libMesh
{
template <typename>
class NumericVector;
}

template <>
InputParameters validParams<RANFSNormalMechanicalContact>();

struct MasterNodeInfo
{
  const Node * master_node;
  std::pair<std::pair<const Elem *, unsigned int>, std::pair<const Elem *, unsigned int>>
      master_elems_and_sides;
};

class RANFSNormalMechanicalContact : public NodeFaceConstraint
{
public:
  RANFSNormalMechanicalContact(const InputParameters & parameters);

  bool shouldApply() override;
  void residualSetup() override;
  void timestepSetup() override;
  bool overwriteSlaveResidual() override;
  void computeSlaveValue(NumericVector<Number> & solution) override;

protected:
  virtual Real computeQpSlaveValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  const MooseEnum _component;
  const unsigned int _mesh_dimension;
  NumericVector<Number> & _residual_copy;

  unsigned int _largest_component;
  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;
  std::unordered_map<dof_id_type, Real> _node_to_lm;
  std::unordered_map<dof_id_type, std::vector<const Elem *>> _node_to_master_elem_sequence;
  Real _lagrange_multiplier;
  PenetrationInfo * _pinfo;
  std::unordered_map<dof_id_type, MasterNodeInfo> _ping_pong_slave_node_to_master_node;
  Real _distance;
  Real _normal_component;
  bool _restrict_master_residual;
  unsigned int _master_index;
};
