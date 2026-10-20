//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "ADFunctorInterface.h"
#include "DittusBoelterHTCFunctor.h"

class SinglePhaseFluidProperties;

/**
 * Supplies a Dittus-Boelter wall heat transfer coefficient as a functor named after this
 * object, for the lumped path-integrated flow segments.
 *
 * The coefficient is published as a functor rather than as a functor material property so
 * that the path-integrated scalar kernels can consume it: those kernels have no current
 * element, and a functor material property requires one to select its per-block lambda.
 */
class ADDittusBoelterHTCUserObject : public GeneralUserObject, public ADFunctorInterface
{
public:
  static InputParameters validParams();
  ADDittusBoelterHTCUserObject(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /// Heat transfer coefficient functor supplied to consumers under this object's name
  DittusBoelterHTCFunctor _htc;
};
