//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Times.h"
#include "FunctorInterface.h"

/**
 * Times created by evaluating a functor
 */
class FunctorTimes : public Times, public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  FunctorTimes(const InputParameters & parameters);
  virtual ~FunctorTimes() = default;

protected:
  virtual void initialize() override;

private:
  /// Functor to evaluate the time to append to the Times vector
  const Moose::Functor<Real> & _functor;
  /// Factor to multiply the functor with
  const Moose::Functor<Real> & _factor;
};
