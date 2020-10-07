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

  virtual Real value(Real t, const Point & p) const override;

protected:

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
};
