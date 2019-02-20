#ifndef HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H
#define HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H

#include "HeatTransferFromTemperature.h"

class HeatTransferFromExternalAppTemperature;

template <>
InputParameters validParams<HeatTransferFromExternalAppTemperature>();

/**
 * Heat transfer connection from temperature provided by an external application
 */
class HeatTransferFromExternalAppTemperature : public HeatTransferFromTemperature
{
public:
  HeatTransferFromExternalAppTemperature(const InputParameters & parameters);

  virtual void check() const override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Name of the function specifying initial condition for wall temperature
  const FunctionName _T_wall_fn_name;
};

#endif /* HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H */
