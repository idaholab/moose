//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "Assembly.h"
#include "SystemBase.h"

class PeridynamicsMesh;

/**
 * Base kernel class for peridynamic models
 */
class PeridynamicsKernelBase : public Kernel
{
public:
  static InputParameters validParams();

  PeridynamicsKernelBase(const InputParameters & parameters);

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
  MooseVariable * _bond_status_var;

  /// Option to use full jacobian including nonlocal constribution or not
  const bool _use_full_jacobian;

  ///@{ Parameters for peridynamic mesh information
  PeridynamicsMesh & _pdmesh;
  const unsigned int _dim;
  const unsigned int _nnodes;
  std::vector<Real> _node_vol;
  std::vector<Real> _dg_vol_frac;
  std::vector<Real> _horizon_radius;
  std::vector<Real> _horizon_vol;
  ///@}

  ///Vector for current bond under undefored configuration
  RealGradient _origin_vec;

  /// Bond status of current bond/edge2
  Real _bond_status;
};
