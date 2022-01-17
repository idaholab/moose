#pragma once

#include "HeatTransferFromTemperature1Phase.h"

/**
 * Heat transfer into 1-phase flow channel from temperature provided by an external application
 */
class HeatTransferFromExternalAppTemperature1Phase : public HeatTransferFromTemperature1Phase
{
public:
  HeatTransferFromExternalAppTemperature1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;
  virtual void initSecondary() override;

  /// Name of the function specifying initial condition for wall temperature
  const FunctionName _T_wall_fn_name;
  /// The type of the wall temperature variable
  FEType _fe_type;

public:
  static InputParameters validParams();
};
