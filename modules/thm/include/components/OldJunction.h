#pragma once

#include "JunctionWithLossesBase.h"

class OldJunction;

template <>
InputParameters validParams<OldJunction>();

/**
 * Junction assuming incompressible flow
 */
class OldJunction : public JunctionWithLossesBase
{
public:
  OldJunction(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// The name of scalar variable that represents the pressure in this junction
  std::string _pressure_var_name;
  /// The name of scalar variable that represents the energy in this junction
  std::string _energy_var_name;
  /// Initial pressure
  Real _initial_P;
  /// Initial volume fraction of vapor
  Real _initial_void_fraction;

  const std::string _total_mfr_in_var_name;
  const std::string _total_int_energy_rate_in_var_name;

  /// Scaling factors for pressure and energy
  std::vector<Real> _scaling_factors;
};
