#pragma once

#include "NumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"

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
                        const Real & nLR_dot_d,
                        std::vector<Real> & FL,
                        std::vector<Real> & FR) const override;

  virtual void calcJacobian(const std::vector<Real> & U1,
                            const std::vector<Real> & U2,
                            const Real & nLR_dot_d,
                            DenseMatrix<Real> & dFL_dUL,
                            DenseMatrix<Real> & dFL_dUR,
                            DenseMatrix<Real> & dFR_dUL,
                            DenseMatrix<Real> & dFR_dUR) const override;

  virtual unsigned int getNumberOfRegions() const override { return 1; }

protected:
  std::vector<Real> computeFlux(const std::vector<Real> & U) const;

  DenseMatrix<Real> computeJacobian(const std::vector<Real> & U) const;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
