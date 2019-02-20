#ifndef HEATTRANSFERFROMTEMPERATURE_H
#define HEATTRANSFERFROMTEMPERATURE_H

#include "HeatTransferBase.h"

class HeatTransferFromTemperature;

template <>
InputParameters validParams<HeatTransferFromTemperature>();

/**
 * Base class for heat transfer connections from temperature
 */
class HeatTransferFromTemperature : public HeatTransferBase
{
public:
  HeatTransferFromTemperature(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /**
   * Adds 1-phase heat transfer kernels
   */
  void addHeatTransferKernels1Phase();
  /**
   * Adds 2-phase heat transfer kernels
   */
  void addHeatTransferKernels2Phase();
};

#endif /* HEATTRANSFERFROMTEMPERATURE_H */
