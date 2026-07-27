//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralFunctorPostprocessor.h"

/**
 * This postprocessor computes an element average of the specified functor
 */
class ElementAverageFunctorPostprocessor : public ElementIntegralFunctorPostprocessor
{
public:
  static InputParameters validParams();

  ElementAverageFunctorPostprocessor(const InputParameters & parameters);

protected:
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;
  virtual Real getValue() const override;

private:
  Real _volume = 0;
};
