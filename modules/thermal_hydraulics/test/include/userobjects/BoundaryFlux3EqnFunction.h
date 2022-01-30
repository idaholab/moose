#pragma once

#include "ADBoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes the 1-phase boundary flux directly from specified functions.
 */
class BoundaryFlux3EqnFunction : public ADBoundaryFluxBase
{
public:
  BoundaryFlux3EqnFunction(const InputParameters & parameters);

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<ADReal> & U1,
                        const RealVectorValue & normal,
                        std::vector<ADReal> & flux) const override;

protected:
  const Function & _rho_fn;
  const Function & _vel_fn;
  const Function & _p_fn;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
