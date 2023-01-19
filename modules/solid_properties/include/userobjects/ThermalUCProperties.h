#pragma once

#include "ThermalSolidProperties.h"


class ThermalUCProperties : public ThermalSolidProperties
{
public:
  static InputParameters validParams();

  ThermalUCProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"



  virtual Real k_from_T(const Real & T) const override;

  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;

  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;

  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;


};

protected:
  /// (constant) density
  const Real & _rho_const;

#pragma GCC diagnostic pop