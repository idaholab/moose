#pragma once

#include "NumericalFlux3EqnBase.h"
#include "SinglePhaseFluidProperties.h"
#include "NaNInterface.h"

/**
 * Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using
 * the HLLC approximate Riemann solver.
 *
 * The approach in the following reference for the 3-D Euler equations was
 * extended to the 1-D, variable-area Euler equations:
 *
 * Batten, P., Leschziner, M. A., & Goldberg, U. C. (1997).
 * Average-state Jacobians and implicit methods for compressible viscous and turbulent flows.
 * Journal of computational physics, 137(1), 38-78.
 */
class NumericalFlux3EqnHLLC : public NumericalFlux3EqnBase, public NaNInterface
{
public:
  NumericalFlux3EqnHLLC(const InputParameters & parameters);

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

  virtual unsigned int getNumberOfRegions() const override { return 4; }

protected:
  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// use frozen acoustic speeds Jacobian approximation
  const bool _use_approximate_jacobian;

public:
  static InputParameters validParams();
};
