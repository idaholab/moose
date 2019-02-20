#ifndef HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H
#define HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H

#include "HeatTransferFromTemperature.h"

class HeatTransferFromSpecifiedTemperature;

template <>
InputParameters validParams<HeatTransferFromSpecifiedTemperature>();

/**
 * Heat transfer connection from a fixed temperature function
 */
class HeatTransferFromSpecifiedTemperature : public HeatTransferFromTemperature
{
public:
  HeatTransferFromSpecifiedTemperature(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// wall temperature function name
  const FunctionName _T_wall_fn_name;
};

#endif /* HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H */
