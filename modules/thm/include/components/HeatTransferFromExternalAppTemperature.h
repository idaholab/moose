#ifndef HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H
#define HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H

#include "HeatTransferBase.h"

class HeatTransferFromExternalAppTemperature;

template <>
InputParameters validParams<HeatTransferFromExternalAppTemperature>();

/**
 * Deprecated class, do not use
 */
class HeatTransferFromExternalAppTemperature : public HeatTransferBase
{
public:
  HeatTransferFromExternalAppTemperature(const InputParameters & parameters);

  virtual bool isTemperatureType() const override { return true; }

protected:
  virtual void check() const override;
};

#endif /* HEATTRANSFERFROMEXTERNALAPPTEMPERATURE_H */
