//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlocalKernel.h"
#include "GrainForceAndTorqueInterface.h"

// Forward Declarations
class GrainTrackerInterface;

class GrainRigidBodyMotionBase : public NonlocalKernel

{
public:
  static InputParameters validParams();

  GrainRigidBodyMotionBase(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  virtual bool globalDoFEnabled(MooseVariableFEBase & /*var*/, dof_id_type /*dof_index*/);

  virtual void precalculateResidual();
  virtual void precalculateJacobian();
  virtual void precalculateOffDiagJacobian(unsigned int jvar);

  virtual void calculateAdvectionVelocity() {}

  /// Variable's local dof indices
  const std::vector<dof_id_type> & _var_dofs;

  /// int label for the Concentration
  unsigned int _c_var;
  /// Variable value for the concentration
  const VariableValue & _c;
  /// Variable gradient for the concentration
  const VariableGradient & _grad_c;
  /// local dof indices of variable c
  const std::vector<dof_id_type> & _c_dofs;

  /// no. of order parameters
  const unsigned int _op_num;
  /// Variable value for the order parameters
  const std::vector<const VariableValue *> _vals;
  const std::vector<unsigned int> _vals_var;
  const std::vector<const VariableGradient *> _grad_vals;

  /// base name specifying type of force density material
  const std::string _base_name;

  /// getting userobject for calculating grain forces and torques
  const GrainForceAndTorqueInterface & _grain_force_torque;
  const std::vector<RealGradient> & _grain_forces;
  const std::vector<RealGradient> & _grain_torques;
  const std::vector<Real> & _grain_force_c_jacobians;
  const std::vector<std::vector<Real>> & _grain_force_eta_jacobians;

  /// constant value corresponding to grain translation
  const Real _mt;
  /// constant value corresponding to grain rotation
  const Real _mr;

  /// grain tracker object
  const GrainTrackerInterface & _grain_tracker;

  /// The grain volumes
  const VectorPostprocessorValue & _grain_volumes;

  /// get the total no. of dofs in the system
  unsigned int _total_dofs;

  /// storing the advection velocity and corresponding jacobian entries calculated in userobjects
  RealGradient _velocity_advection;
  RealGradient _velocity_advection_jacobian;
  /// obtain the active grain ids
  std::vector<unsigned int> _grain_ids;
};
