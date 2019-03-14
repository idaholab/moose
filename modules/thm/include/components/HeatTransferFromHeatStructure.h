#ifndef HEATTRANSFERFROMHEATSTRUCTURE_H
#define HEATTRANSFERFROMHEATSTRUCTURE_H

#include "HeatTransferBase.h"

class HeatTransferFromHeatStructure;

template <>
InputParameters validParams<HeatTransferFromHeatStructure>();

/**
 * Deprecated class, do not use.
 */
class HeatTransferFromHeatStructure : public HeatTransferBase
{
public:
  HeatTransferFromHeatStructure(const InputParameters & parameters);

  virtual bool isTemperatureType() const override { return true; }

protected:
  virtual void check() const override;
};

#endif /* HEATTRANSFERFROMHEATSTRUCTURE_H */
