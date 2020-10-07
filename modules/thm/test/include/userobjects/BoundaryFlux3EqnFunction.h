#pragma once

#include "BoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes the 1-phase boundary flux directly from specified functions.
 */
class BoundaryFlux3EqnFunction : public BoundaryFluxBase
{
public:
  BoundaryFlux3EqnFunction(const InputParameters & parameters);

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & U1,
                        const RealVectorValue & normal,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & U1,
                            const RealVectorValue & normal,
                            DenseMatrix<Real> & jac1) const override;

protected:
  const Function & _rho_fn;
  const Function & _vel_fn;
  const Function & _p_fn;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
