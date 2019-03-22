#ifndef HEATTRANSFERFROMHEATFLUX1PHASE_H
#define HEATTRANSFERFROMHEATFLUX1PHASE_H

#include "HeatTransfer1PhaseBase.h"

class HeatTransferFromHeatFlux1Phase;

template <>
InputParameters validParams<HeatTransferFromHeatFlux1Phase>();

/**
 * Heat transfer specified by heat flux going into 1-phase flow channel
 */
class HeatTransferFromHeatFlux1Phase : public HeatTransfer1PhaseBase
{
public:
  HeatTransferFromHeatFlux1Phase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  virtual bool isTemperatureType() const override;

protected:
  /// wall heat flux function name
  const FunctionName _q_wall_fn_name;
};

#endif /* HEATTRANSFERFROMHEATFLUX1PHASE_H */
