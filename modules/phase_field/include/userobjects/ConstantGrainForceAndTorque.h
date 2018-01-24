/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSTANTGRAINFORCEANDTORQUE_H
#define CONSTANTGRAINFORCEANDTORQUE_H

#include "GeneralUserObject.h"
#include "GrainForceAndTorqueInterface.h"

// Forward Declarations
class ConstantGrainForceAndTorque;

template <>
InputParameters validParams<ConstantGrainForceAndTorque>();

/**
 * This class is here to get the force and torque acting on a grain
 */
class ConstantGrainForceAndTorque : public GrainForceAndTorqueInterface, public GeneralUserObject
{
public:
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

#endif // CONSTANTGRAINFORCEANDTORQUE_H
