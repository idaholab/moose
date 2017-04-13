/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVHLLCINTERNALSIDEFLUX_H
#define CNSFVHLLCINTERNALSIDEFLUX_H

#include "InternalSideFluxBase.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVHLLCInternalSideFlux;

template <>
InputParameters validParams<CNSFVHLLCInternalSideFlux>();

/**
 * A user object that computes internal side flux using the HLLC approximate Riemann solver
 *
 * Reference article
 *
 * Batten, P., Leschziner, M. A., & Goldberg, U. C. (1997).
 * Average-state Jacobians and implicit methods for compressible viscous and turbulent flows.
 * Journal of computational physics, 137(1), 38-78.
 */
class CNSFVHLLCInternalSideFlux : public InternalSideFluxBase
{
public:
  CNSFVHLLCInternalSideFlux(const InputParameters & parameters);
  virtual ~CNSFVHLLCInternalSideFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const;

protected:
  const SinglePhaseFluidProperties & _fp;
};

#endif
