#ifndef BOUNDARYFLUX3EQNFREEINFLOW_H
#define BOUNDARYFLUX3EQNFREEINFLOW_H

#include "BoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

class BoundaryFlux3EqnFreeInflow;

template <>
InputParameters validParams<BoundaryFlux3EqnFreeInflow>();

/**
 * Computes the inflow boundary flux directly for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnFreeInflow : public BoundaryFluxBase
{
public:
  BoundaryFlux3EqnFreeInflow(const InputParameters & parameters);

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
  const Real _rho_inf;
  const Real _vel_inf;
  const Real _p_inf;

  const SinglePhaseFluidProperties & _fp;
};

#endif
