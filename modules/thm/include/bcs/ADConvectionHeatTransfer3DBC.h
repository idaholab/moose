#pragma once

#include "ADIntegratedBC.h"

class ADHeatTransferFromHeatStructure3D1PhaseUserObject;

/**
 * Convective boundary condition from a single-phase flow channel for a 3D heat structure.
 */
class ADConvectionHeatTransfer3DBC : public ADIntegratedBC
{
public:
  ADConvectionHeatTransfer3DBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// User object that computes the heat transfer data (fluid temperature and heat transfer coefficient)
  const ADHeatTransferFromHeatStructure3D1PhaseUserObject & _ht_uo;

public:
  static InputParameters validParams();
};
