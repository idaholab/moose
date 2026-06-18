//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPostprocessor.h"

/**
 * Postprocessor that computes the volume average of a functor over the elements. The functor is
 * evaluated with element spatial arguments and does not use quadrature points.
 */
class ElementFVAverageValue : public ElementPostprocessor
{
public:
  ElementFVAverageValue(const InputParameters & params);

  static InputParameters validParams();

  void initialize() override;
  void execute() override;
  void finalize() override;
  void threadJoin(const UserObject & other_thread_uo) override;

  Real getValue() const override;

protected:
  /// The functor being averaged
  const Moose::Functor<Real> & _functor;
  /// Running volume integral of the functor
  Real _integral = 0;
  /// Running total element volume
  Real _volume = 0;
  /// The resulting volume average (integral / volume)
  Real _avg = 0;
};
