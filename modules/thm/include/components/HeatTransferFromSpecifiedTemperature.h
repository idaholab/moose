#ifndef HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H
#define HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H

#include "HeatTransferBase.h"

class HeatTransferFromSpecifiedTemperature;

template <>
InputParameters validParams<HeatTransferFromSpecifiedTemperature>();

/**
 * Deprecated class, do not use.
 */
class HeatTransferFromSpecifiedTemperature : public HeatTransferBase
{
public:
  HeatTransferFromSpecifiedTemperature(const InputParameters & parameters);

  virtual bool isTemperatureType() const override { return true; }

protected:
  virtual void check() const override;
};

#endif /* HEATTRANSFERFROMSPECIFIEDTEMPERATURE_H */
