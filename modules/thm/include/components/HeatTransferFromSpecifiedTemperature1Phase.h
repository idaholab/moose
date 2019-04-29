#pragma once

#include "HeatTransferFromTemperature1Phase.h"

class HeatTransferFromSpecifiedTemperature1Phase;

template <>
InputParameters validParams<HeatTransferFromSpecifiedTemperature1Phase>();

/**
 * Heat transfer connection from a fixed temperature function for 1-phase flow
 */
class HeatTransferFromSpecifiedTemperature1Phase : public HeatTransferFromTemperature1Phase
{
public:
  HeatTransferFromSpecifiedTemperature1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// wall temperature function name
  const FunctionName _T_wall_fn_name;
};
