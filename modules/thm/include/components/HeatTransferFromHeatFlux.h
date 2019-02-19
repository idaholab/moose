#ifndef HEATTRANSFERFROMHEATFLUX_H
#define HEATTRANSFERFROMHEATFLUX_H

#include "HeatTransferBase.h"

class HeatTransferFromHeatFlux;

template <>
InputParameters validParams<HeatTransferFromHeatFlux>();

/**
 * Base class for heat transfer connections from heat flux
 */
class HeatTransferFromHeatFlux : public HeatTransferBase
{
public:
  HeatTransferFromHeatFlux(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /**
   * Adds 1-phase objects
   */
  void addMooseObjects1Phase();
  /**
   * Adds 2-phase objects
   */
  void addMooseObjects2Phase();

  /// wall heat flux function name
  const FunctionName _q_wall_fn_name;
};

#endif /* HEATTRANSFERFROMHEATFLUX_H */
