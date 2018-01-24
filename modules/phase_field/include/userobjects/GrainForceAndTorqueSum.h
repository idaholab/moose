/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINFORCEANDTORQUESUM_H
#define GRAINFORCEANDTORQUESUM_H

#include "GrainForceAndTorqueInterface.h"
#include "GeneralUserObject.h"

// Forward Declarations
class GrainForceAndTorqueSum;

template <>
InputParameters validParams<GrainForceAndTorqueSum>();

/**
 * This class is here to get the force and torque acting on a grain
 * from different userobjects and sum them all
 */
class GrainForceAndTorqueSum : public GrainForceAndTorqueInterface, public GeneralUserObject
{
public:
  GrainForceAndTorqueSum(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute(){};
  virtual void finalize(){};

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<Real> & getForceCJacobians() const;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const;

protected:
  /// Vector of userobjects providing forces and torques acting on grains
  std::vector<UserObjectName> _sum_objects;
  /// Total no. of userobjects that provides forces and torques acting on grains
  unsigned int _num_forces;
  unsigned int _grain_num;

  std::vector<const GrainForceAndTorqueInterface *> _sum_forces;

  ///@{ providing grain forces, torques and their jacobians w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<Real> _c_jacobians;
  std::vector<std::vector<Real>> _eta_jacobians;
  ///@}
};

#endif // GRAINFORCEANDTORQUESUM_H
