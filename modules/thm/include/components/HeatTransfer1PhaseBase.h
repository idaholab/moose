#ifndef HEATTRANSFER1PHASEBASE_H
#define HEATTRANSFER1PHASEBASE_H

#include "HeatTransferBase.h"

class HeatTransfer1PhaseBase;

template <>
InputParameters validParams<HeatTransfer1PhaseBase>();

/**
 * Base class for heat transfer connections to 1-phase flow channels
 */
class HeatTransfer1PhaseBase : public HeatTransferBase
{
public:
  HeatTransfer1PhaseBase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

  /**
   * Returns 1-phase wall heat transfer coefficient name
   *
   * @return The name of the 1-phase wall heat transfer coefficient variable
   */
  const MaterialPropertyName & getWallHeatTransferCoefficient1PhaseName() const;

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// 1-phase wall heat transfer coefficient name
  MaterialPropertyName _Hw_1phase_name;
};

#endif /* HEATTRANSFER1PHASEBASE_H */
