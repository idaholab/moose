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

#include "libmesh/coupling_matrix.h"

#include <vector>
#include <unordered_map>

class PenetrationInfo;

namespace libMesh
{
template <typename>
class NumericVector;
}

class RANFSNormalMechanicalContact : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  RANFSNormalMechanicalContact(const InputParameters & parameters);

  bool shouldApply() override;
  void residualSetup() override;
  void timestepSetup() override;
  void initialSetup() override;
  bool overwriteSecondaryResidual() override;
  void computeSecondaryValue(NumericVector<Number> & solution) override;

protected:
  virtual Real computeQpSecondaryValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  const MooseEnum _component;
  const unsigned int _mesh_dimension;
  NumericVector<Number> & _residual_copy;

  unsigned int _largest_component;
  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;
  std::unordered_map<dof_id_type, Real> _node_to_contact_lm;
  std::unordered_map<dof_id_type, Real> _node_to_tied_lm;
  std::unordered_map<dof_id_type, std::vector<const Elem *>> _node_to_primary_elem_sequence;
  Real _contact_lm;
  Real _tied_lm;
  PenetrationInfo * _pinfo;
  std::unordered_map<dof_id_type, const Node *> _ping_pong_secondary_node_to_primary_node;
  Real _distance;
  bool _tie_nodes;
  unsigned int _primary_index;
  RealVectorValue _res_vec;
  const Node * _nearest_node;
  std::vector<std::unordered_map<dof_id_type, Number>> _dof_number_to_value;
  CouplingMatrix _disp_coupling;
  const bool _ping_pong_protection;
};
