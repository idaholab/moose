/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVHLLCINFLOWOUTFLOWBOUNDARYFLUX_H
#define CNSFVHLLCINFLOWOUTFLOWBOUNDARYFLUX_H

#include "BoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVHLLCInflowOutflowBoundaryFlux;

template <>
InputParameters validParams<CNSFVHLLCInflowOutflowBoundaryFlux>();

/**
 * A user object that computes inflow/outflow boundary flux using the HLLC approximate Riemann
 * solver
 */
class CNSFVHLLCInflowOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  CNSFVHLLCInflowOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~CNSFVHLLCInflowOutflowBoundaryFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & uvec1,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & uvec1,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1) const;

protected:
  const SinglePhaseFluidProperties & _fp;

  Real _inf_rho;
  Real _inf_uadv;
  Real _inf_vadv;
  Real _inf_wadv;
  Real _inf_pres;
};

#endif
