#ifndef HEATTRANSFERFROMHEATFLUX_H
#define HEATTRANSFERFROMHEATFLUX_H

#include "HeatTransferBase.h"

class HeatTransferFromHeatFlux;

template <>
InputParameters validParams<HeatTransferFromHeatFlux>();

/**
 * Deprecated class, do not use.
 */
class HeatTransferFromHeatFlux : public HeatTransferBase
{
public:
  HeatTransferFromHeatFlux(const InputParameters & parameters);

  virtual bool isTemperatureType() const override { return false; }

protected:
  virtual void check() const override;
};

#endif /* HEATTRANSFERFROMHEATFLUX_H */
