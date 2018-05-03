//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef KERNELBASEPD_H
#define KERNELBASEPD_H

#include "Kernel.h"
#include "Assembly.h"
#include "SystemBase.h"

class MeshBasePD;
class KernelBasePD;

template <>
InputParameters validParams<KernelBasePD>();

/**
 * Base kernel class for peridynamic models
 */
class KernelBasePD : public Kernel
{
public:
  KernelBasePD(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual Real computeQpResidual() override { return 0.0; }

protected:
  /**
   * Function to compute local contribution to the residual at the current nodes
   */
  virtual void computeLocalResidual() = 0;

  /**
   * Function to compute nonlocal contribution to the residual at the current nodes
   */
  virtual void computeNonlocalResidual(){};

  /**
   * Function to compute local contribution to the diagonal Jacobian at the current nodes
   */
  virtual void computeLocalJacobian(){};

  /**
   * Function to precalculate data which will be used in the derived classes
   */
  virtual void prepare();

  /// Bond_status variable
  const MooseVariableFEBase & _bond_status_var;

  /// Option to use full jacobian including nonlocal constribution or not
  const bool _use_full_jacobian;

  ///@{ Parameters for peridynamic mesh information
  MeshBasePD & _pdmesh;
  const unsigned int _dim;
  const unsigned int _nnodes;
  std::vector<Node *> _nodes_ij;
  std::vector<Real> _vols_ij;
  std::vector<Real> _dg_bond_vsum_ij;
  std::vector<Real> _dg_node_vsum_ij;
  std::vector<Real> _horizons_ij;
  ///@}

  ///Vector for current bond under undefored configuration
  RealGradient _origin_vec_ij;

  /// Bond status of current bond/edge2
  Real _bond_status_ij;
};

#endif // PDKERNELBASE_H
