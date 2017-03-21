/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVHLLCSLIPWALLBOUNDARYFLUX_H
#define CNSFVHLLCSLIPWALLBOUNDARYFLUX_H

#include "BoundaryFluxBase.h"
#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVHLLCSlipBoundaryFlux;

template <>
InputParameters validParams<CNSFVHLLCSlipBoundaryFlux>();

/**
 * A user object that computes the slip boundary flux using the HLLC approximate Riemann solver
 */
class CNSFVHLLCSlipBoundaryFlux : public BoundaryFluxBase
{
public:
  CNSFVHLLCSlipBoundaryFlux(const InputParameters & parameters);
  virtual ~CNSFVHLLCSlipBoundaryFlux();

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
  const BCUserObject & _bc_uo;
  const SinglePhaseFluidProperties & _fp;
};

#endif
