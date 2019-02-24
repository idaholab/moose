//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FLUIDPROPERTIES_H
#define FLUIDPROPERTIES_H

#include "ThreadedGeneralUserObject.h"

// Forward Declarations
class FluidProperties;

typedef DualNumber<Real, NumberArray<3, Real>> FPDualReal;

template <>
InputParameters validParams<FluidProperties>();

class FluidProperties : public ThreadedGeneralUserObject
{
public:
  FluidProperties(const InputParameters & parameters);
  virtual ~FluidProperties();

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}

  virtual void threadJoin(const UserObject &) final {}
  virtual void subdomainSetup() final {}

protected:
  /// Conversion of temperature from Celsius to Kelvin
  const Real _T_c2k;
  /// Flag to set unimplemented Jacobian entries to zero
  const bool _allow_imperfect_jacobians;
};

#endif /* FLUIDPROPERTIES_H */
