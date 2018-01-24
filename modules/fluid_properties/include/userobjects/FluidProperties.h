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

#include "GeneralUserObject.h"

// Forward Declarations
class FluidProperties;

template <>
InputParameters validParams<FluidProperties>();

class FluidProperties : public GeneralUserObject
{
public:
  FluidProperties(const InputParameters & parameters);
  virtual ~FluidProperties();

  virtual void execute() final {}
  virtual void initialize() final {}
  virtual void finalize() final {}
};

#endif /* FLUIDPROPERTIES_H */
