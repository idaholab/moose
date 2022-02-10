//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "ADCubicTransition.h"
#include "ADWeightedTransition.h"

#include "ADReal.h"

/**
 * Class for testing objects derived from ADSmoothTransition
 */
class ADSmoothTransitionTestMaterial : public Material
{
public:
  ADSmoothTransitionTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  /**
   * Computes left function
   *
   * @param[in] x   Point at which to evaluate the function
   */
  ADReal f1(const ADReal & x) const;
  /**
   * Computes right function
   *
   * @param[in] x   Point at which to evaluate the function
   */
  ADReal f2(const ADReal & x) const;
  /**
   * Computes left function derivative
   *
   * @param[in] x   Point at which to evaluate the function
   */
  ADReal df1dx(const ADReal & x) const;
  /**
   * Computes right function derivative
   *
   * @param[in] x   Point at which to evaluate the function
   */
  ADReal df2dx(const ADReal & x) const;

  /// Type of transition
  const MooseEnum & _transition_type;
  /// Variable the transition depends upon
  const ADVariableValue & _var;
  /// Center point of transition
  const ADReal _center;
  /// Width of transition
  const ADReal _width;
  /// Cubic transition
  ADCubicTransition _cubic_transition;
  /// Weighted transition_base
  const ADWeightedTransition _weighted_transition;
  /// Material property created using the transition_base
  ADMaterialProperty<Real> & _matprop;

public:
  static InputParameters validParams();
};
