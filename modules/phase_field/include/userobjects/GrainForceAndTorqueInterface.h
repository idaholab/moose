/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINFORCEANDTORQUEINTERFACE_H
#define GRAINFORCEANDTORQUEINTERFACE_H

#include "MooseObject.h"

/**
 * This class provides interface for extracting the forces and torques computed in other UserObjects
 */
class GrainForceAndTorqueInterface
{
public:
  virtual const std::vector<RealGradient> & getForceValues() const = 0;
  virtual const std::vector<RealGradient> & getTorqueValues() const = 0;
  virtual const std::vector<Real> & getForceCJacobians() const = 0;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const = 0;
};

#endif // GRAINFORCEANDTORQUEINTERFACE_H
