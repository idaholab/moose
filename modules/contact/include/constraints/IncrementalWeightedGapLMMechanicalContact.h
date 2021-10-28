//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeWeightedGapLMMechanicalContact.h"

class IncrementalWeightedGapLMMechanicalContact : public ComputeWeightedGapLMMechanicalContact
{
public:
  static InputParameters validParams();

  IncrementalWeightedGapLMMechanicalContact(const InputParameters & parameters);
  using ComputeWeightedGapLMMechanicalContact::computeResidual;
  void computeResidual(Moose::MortarType mortar_type) override;
  void residualSetup() override;
  void initialSetup() override;
  void post() override;
  void
  incorrectEdgeDroppingPost(const std::unordered_set<const Node *> & inactive_lm_nodes) override;

protected:
  void buildIncrements();
  void enforceConstraintOnDof(const DofObject * const dof) override;

  NumericVector<Number> & _D;
  SparseMatrix<Number> & _M;

  const MooseVariable & _disp_x_var;
  const MooseVariable & _disp_y_var;
  const MooseVariable * const _disp_z_var;

  const bool _use_vertices;
  const unsigned int _n_disp;

  std::unique_ptr<NumericVector<Number>> _delta_d;
  std::unique_ptr<NumericVector<Number>> _sub_D;

  std::vector<unsigned int> _secondary_lower_to_ip;
  std::vector<unsigned int> _primary_lower_to_ip;

  DenseVector<Real> _local_D;
  DenseMatrix<Real> _local_M;

  std::vector<dof_id_type> _rows;
  std::vector<dof_id_type> _cols;

  std::unordered_map<dof_id_type, std::array<dof_id_type, LIBMESH_DIM>>
      _node_id_to_mortar_disp_indices;
};
