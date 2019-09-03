#pragma once

#include "VolumeJunctionOldBase.h"

class VolumeJunctionOld;

template <>
InputParameters validParams<VolumeJunctionOld>();

/**
 * Junction with a non-zero volume
 */
class VolumeJunctionOld : public VolumeJunctionOldBase
{
public:
  VolumeJunctionOld(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual const VariableName & getPressureVariableName() const { return _pressure_var_name; }
  virtual const VariableName & getDensityVariableName() const { return _rho_var_name; }
  virtual const VariableName & getInternalEnergyDensityVariableName() const
  {
    return _rhoe_var_name;
  }

protected:
  virtual void check() const override;

  const VariableName _rho_var_name;
  const VariableName _rhoe_var_name;
  const VariableName _vel_var_name;
  const VariableName _pressure_var_name;
  const VariableName _energy_var_name;
  const VariableName _total_mfr_in_var_name;
};
