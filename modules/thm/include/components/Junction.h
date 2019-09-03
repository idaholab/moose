#pragma once

#include "FlowJunction.h"

class Junction;

template <>
InputParameters validParams<Junction>();

/**
 * Compressible junction
 *
 * Currently, this adds one constraint to the nonlinear system to enforce mass
 * balance at the junction. This equation is associated with a new degree of
 * freedom: the entropy of the junction. This variable does not require an initial
 * condition, but it does require an initial guess for nonlinear iteration to
 * prevent NaNs being generated from the equation of state and thus terminating
 * simulation. Currently, this initial guess is computed from the global scalar
 * values for the initial temperature and pressure. However, this assumes that
 * the resulting entropy value would be physical in all junctions, which later
 * may be found to be false; in this case, a special IC will need to be written
 * for this variable. An auxiliary computation gives the stagnation enthalpy of
 * the junction. These junction quantities may appear in the boundary flux for a
 * given connected flow channel, depending on flow conditions.
 */
class Junction : public FlowJunction
{
public:
  Junction(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  const VariableName _entropy_var_name;
  const UserObjectName _H_junction_uo_name;

  const Real _initial_p;
  const Real _initial_T;

  const Real _scaling_factor_s_junction;

  /// form loss coefficients for each connection
  std::vector<Real> _K;
};
