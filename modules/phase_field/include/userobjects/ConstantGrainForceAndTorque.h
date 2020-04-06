//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "GrainForceAndTorqueInterface.h"

// Forward Declarations

/**
 * This class is here to get the force and torque acting on a grain
 */
class ConstantGrainForceAndTorque : public GrainForceAndTorqueInterface, public GeneralUserObject
{
public:
  static InputParameters validParams();

  ConstantGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute() {}
  virtual void finalize() {}

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<Real> & getForceCJacobians() const;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const;

protected:
  /// Applied force on particles, size should be 3 times no. of grains
  std::vector<Real> _F;
  /// Applied torque on particles, size should be 3 times no. of grains
  std::vector<Real> _M;

  unsigned int _grain_num;
  unsigned int _ncomp;

  ///@{ providing grain forces, torques and their jacobians w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<Real> _c_jacobians;
  std::vector<std::vector<Real>> _eta_jacobians;
  ///@}
};
