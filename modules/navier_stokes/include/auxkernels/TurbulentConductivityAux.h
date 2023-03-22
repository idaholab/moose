

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "INSFVVelocityVariable.h"
/**
 * Computes the turbuent viscosity for the k-Epsilon model.
 * Implements two near-wall treatements: equilibrium and non-equilibrium wall functions.
 */
class TurbulentConductivityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TurbulentConductivityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  // virtual void meshChanged() override;

  // Local method to find friction velocty
  // Note: this method may be to need reimplemented for each new turbulent model
  // ADReal findUStarLocalMethod(const ADReal & u, const Real & dist);

  /// Specific at Constant Pressure
  const Moose::Functor<ADReal> & _cp;

  /// Turbulent Pradtl
  const Moose::Functor<ADReal> & _Pr_t;

  /// Turbulent viscosity
  const Moose::Functor<ADReal> & _mu_t;
};
