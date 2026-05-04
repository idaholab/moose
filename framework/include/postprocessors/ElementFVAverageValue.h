//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPostprocessor.h"

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
  const Moose::Functor<Real> & _functor;
  Real _integral = 0;
  Real _volume = 0;
  Real _avg = 0;
};
