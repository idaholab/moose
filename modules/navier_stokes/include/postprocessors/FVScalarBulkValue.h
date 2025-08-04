//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * Average a variable value using a weight mask given by a material property.
 */
class FVScalarBulkValue : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  FVScalarBulkValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// variable to average
  const Moose::Functor<Real> & _scalar;

  /// weight mask for averaging
  const Moose::Functor<Real> & _u;

  ///@{ integtals for computimng the average - computed in execute()
  Real _var_integral;
  Real _weight_integral;
  ///@}
};
