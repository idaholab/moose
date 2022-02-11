//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterfaceTHM.h"
#include "CubicTransition.h"
#include "WeightedTransition.h"

/**
 * Class for testing objects derived from SmoothTransition
 */
class SmoothTransitionTestMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  SmoothTransitionTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  Real f1(const Real & x) const;
  Real f2(const Real & x) const;
  Real df1dx(const Real & x) const;
  Real df2dx(const Real & x) const;

  const MooseEnum & _transition_type;

  const VariableValue & _var;

  CubicTransition _cubic_transition;
  const WeightedTransition _weighted_transition;

  MaterialProperty<Real> & _matprop;
  MaterialProperty<Real> & _dmatprop_dvar;

public:
  static InputParameters validParams();
};
