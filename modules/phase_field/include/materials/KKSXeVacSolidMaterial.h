//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations

class KKSXeVacSolidMaterial : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  KKSXeVacSolidMaterial(const InputParameters & parameters);

protected:
  virtual unsigned int expectedNumArgs();

  virtual Real computeF();
  virtual Real computeDF(unsigned int arg);
  virtual Real computeD2F(unsigned int arg1, unsigned int arg2);

private:
  /// Temperature in [K]
  const Real _T;

  /// Atomic volume in [Ang^3]
  const Real _Omega;

  /// Bolzmann constant
  const Real _kB;

  /// Formation energy of a tri-vacancy in UO2
  const Real _Efv;

  /// Formation energy of a Xenon Atom in a tri-vacancy
  /// (TODO: if cmg>cmv consider interstitial Xe)
  const Real _Efg;

  const VariableValue & _cmg;
  unsigned int _cmg_var;
  const VariableValue & _cmv;
  unsigned int _cmv_var;

  // helper function to return a well defined c*log(c)
  Real cLogC(Real c);
};
