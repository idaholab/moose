/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVFREEINFLOWBOUNDARYFLUX_H
#define CNSFVFREEINFLOWBOUNDARYFLUX_H

#include "BoundaryFluxBase.h"
#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVFreeInflowBoundaryFlux;

template <>
InputParameters validParams<CNSFVFreeInflowBoundaryFlux>();

/**
 * A user object that computes the inflow boundary flux
 */
class CNSFVFreeInflowBoundaryFlux : public BoundaryFluxBase
{
public:
  CNSFVFreeInflowBoundaryFlux(const InputParameters & parameters);
  virtual ~CNSFVFreeInflowBoundaryFlux();

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
