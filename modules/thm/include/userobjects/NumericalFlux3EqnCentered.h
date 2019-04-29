#pragma once

#include "NumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"

class NumericalFlux3EqnCentered;

template <>
InputParameters validParams<NumericalFlux3EqnCentered>();

/**
 * Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using
 * a centered average of the left and right side fluxes
 */
class NumericalFlux3EqnCentered : public NumericalFlux3EqnBase
{
public:
  NumericalFlux3EqnCentered(const InputParameters & parameters);

  virtual void calcFlux(const std::vector<Real> & U1,
                        const std::vector<Real> & U2,
                        const RealVectorValue & normal,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(const std::vector<Real> & U1,
                            const std::vector<Real> & U2,
                            const RealVectorValue & normal,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const override;

  virtual unsigned int getNumberOfRegions() const override { return 1; }

protected:
  std::vector<Real> computeFlux(const std::vector<Real> & U) const;

  DenseMatrix<Real> computeJacobian(const std::vector<Real> & U) const;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};
