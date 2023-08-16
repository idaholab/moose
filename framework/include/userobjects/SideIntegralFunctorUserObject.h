//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralUserObject.h"
#include "NonADFunctorInterface.h"

/**
 * Computes a side integral of the specified functor.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralFunctorUserObject : public SideIntegralUserObject, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  SideIntegralFunctorUserObject(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Functor to integrate
  const Moose::Functor<Real> & _functor;
};
