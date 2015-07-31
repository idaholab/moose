/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/


#ifndef GRAINFORCEANDTORQUEINTERFACE_H
#define GRAINFORCEANDTORQUEINTERFACE_H

#include "MooseObject.h"

/**
 * This class is here to get the force and torque acting on a grain
 */
class GrainForceAndTorqueInterface
{
public:
  virtual const std::vector<RealGradient> & getForceValues() const = 0;
  virtual const std::vector<RealGradient> & getTorqueValues() const = 0;
  virtual const std::vector<RealGradient> & getForceDerivatives() const = 0;
  virtual const std::vector<RealGradient> & getTorqueDerivatives() const = 0;
};

#endif //GRAINFORCEANDTORQUEINTERFACE_H
