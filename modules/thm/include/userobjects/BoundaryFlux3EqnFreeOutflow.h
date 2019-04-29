#pragma once

#include "BoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

class BoundaryFlux3EqnFreeOutflow;

template <>
InputParameters validParams<BoundaryFlux3EqnFreeOutflow>();

/**
 * Computes the outflow boundary flux directly for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnFreeOutflow : public BoundaryFluxBase
{
public:
  BoundaryFlux3EqnFreeOutflow(const InputParameters & parameters);

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
  const SinglePhaseFluidProperties & _fp;
};
