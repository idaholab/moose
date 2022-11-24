//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfacePostprocessor.h"

/**
 * This postprocessor add generel capabilities to the InterfacePostprocessor to compute an
 * integral over an interface. To actually compute an integral one must derive from this class,
 * specialize computeQpIntegral() and give access to either a varaible or a material property
 **/
class InterfaceIntegralPostprocessor : public InterfacePostprocessor
{
public:
  static InputParameters validParams();

  InterfaceIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

  static MooseEnum integralTypeOptions();

protected:
  virtual Real computeQpIntegral() = 0; // MUST BE OVERRIDDEN
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};
