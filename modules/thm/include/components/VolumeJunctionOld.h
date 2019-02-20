#ifndef VOLUMEJUNCTIONOLD_H
#define VOLUMEJUNCTIONOLD_H

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

  virtual const std::string & getPressureVariableName() const { return _pressure_var_name; }
  virtual const std::string & getDensityVariableName() const { return _rho_var_name; }
  virtual const std::string & getInternalEnergyDensityVariableName() const
  {
    return _rhoe_var_name;
  }

protected:
  virtual void check() const override;

  const std::string _rho_var_name;
  const std::string _rhoe_var_name;
  const std::string _vel_var_name;
  const std::string _pressure_var_name;
  const std::string _energy_var_name;
  const std::string _total_mfr_in_var_name;
};

#endif /* VOLUMEJUNCTION_H */
