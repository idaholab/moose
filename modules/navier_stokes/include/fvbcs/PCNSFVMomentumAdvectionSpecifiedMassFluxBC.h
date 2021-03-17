#pragma once

#include "PNSFVMassSpecifiedMassFluxBC.h"

/**
 * This boundary provides the advective flux of mass across a boundary
 * given specified mass fluxes
 */
class PCNSFVMomentumAdvectionSpecifiedMassFluxBC : public PNSFVMassSpecifiedMassFluxBC
{
public:
  PCNSFVMomentumAdvectionSpecifiedMassFluxBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;
  void computeMemberData() override;

  const ADMaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _eps;
  const unsigned int _index;
  ADRealVectorValue _velocity;
};
