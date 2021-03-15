#pragma once

#include "FVFluxBC.h"

class Function;

/**
 * This boundary provides the advective flux of mass across a boundary
 * given specified mass fluxes
 */
class PNSFVMassSpecifiedMassFluxBC : public FVFluxBC
{
public:
  PNSFVMassSpecifiedMassFluxBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  const Function & _superficial_rhou;
  const Function * const _superficial_rhov;
  const Function * const _superficial_rhow;
};
