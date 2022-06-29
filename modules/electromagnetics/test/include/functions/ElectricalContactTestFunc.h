//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

/**
 *  Analytical solution function to test the ElectrostaticContactCondition
 *  interface kernel. Constants are taken from the materials (graphite and
 *  stainless steel) used within the test. Defaults are taken at 300 K.
 */
class ElectricalContactTestFunc : public Function
{
public:
  static InputParameters validParams();

  ElectricalContactTestFunc(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Function used to calculate two block test case analytic solution
  Real twoBlockFunction(Real t, const Point & p) const;

  /// Function used to calculate three block test case analytic solution
  Real threeBlockFunction(Real t, const Point & p) const;

  /// Electrical conductivity property for graphite
  const Real & _electrical_conductivity_graphite;

  /// Electrical conductivity property for stainless steel
  const Real & _electrical_conductivity_stainless_steel;

  /// Geometric mean of the hardness of graphite and stainless steel
  const Real & _mean_hardness;

  /// User-supplied mechanical pressure
  const Real & _mechanical_pressure;

  /// Contact conductance property for the tested interface
  const Real & _electrical_contact_conductance;

  /**
   *  MooseEnum to determine which part of the analytic solution
   *  needs to be enabled (Stainless Steel vs. Graphite)
   */
  const MooseEnum & _domain;

  /// Boolean to determine if test function is being used in three block test case
  const bool & _is_three_block;

  /**
   * MooseEnum to determine which stainless steel region needs to be enabled in
   * the three block analytic solution.
   */
  const MooseEnum & _side;

private:
  /**
   * Enum used in comparisons with _domain. Enum-to-enum comparisons are a bit
   * more lightweight, so we should create another enum with the possible choices.
   */
  enum DomainEnum
  {
    STAINLESS_STEEL,
    GRAPHITE
  };
};
