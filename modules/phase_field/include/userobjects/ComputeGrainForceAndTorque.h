//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ShapeElementUserObject.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class GrainTrackerInterface;

/**
 * This class is here to get the force and torque acting on a grain
 */
class ComputeGrainForceAndTorque : public DerivativeMaterialInterface<ShapeElementUserObject>,
                                   public GrainForceAndTorqueInterface
{
public:
  static InputParameters validParams();

  ComputeGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void executeJacobian(unsigned int jvar);
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<Real> & getForceCJacobians() const;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const;

protected:
  unsigned int _qp;

  VariableName _c_name;
  unsigned int _c_var;
  /// material property that provides force density
  MaterialPropertyName _dF_name;
  const MaterialProperty<std::vector<RealGradient>> & _dF;
  /// material property that provides jacobian of force density with respect to c
  const MaterialProperty<std::vector<RealGradient>> & _dFdc;
  /// no. of order parameters
  unsigned int _op_num;
  /// provide UserObject for calculating grain volumes and centers
  const GrainTrackerInterface & _grain_tracker;
  unsigned int _grain_num;
  unsigned int _ncomp;

  std::vector<unsigned int> _vals_var;
  std::vector<VariableName> _vals_name;
  std::vector<const MaterialProperty<std::vector<Real>> *> _dFdgradeta;
  std::vector<const MaterialProperty<Real> *> _test_derivatives;

  ///@{ providing grain forces, torques and their jacobians w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;

  /// vector storing grain force and torque values
  std::vector<Real> _force_torque_store;
  /// vector storing jacobian of grain force and torque values
  std::vector<Real> _force_torque_c_jacobian_store;
  std::vector<std::vector<Real>> _force_torque_eta_jacobian_store;

  unsigned int _total_dofs;
};
