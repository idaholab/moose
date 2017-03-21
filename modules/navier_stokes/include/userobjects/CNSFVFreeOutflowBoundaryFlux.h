/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVFREEOUTFLOWBOUNDARYFLUX_H
#define CNSFVFREEOUTFLOWBOUNDARYFLUX_H

#include "BoundaryFluxBase.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVFreeOutflowBoundaryFlux;

template <>
InputParameters validParams<CNSFVFreeOutflowBoundaryFlux>();

/**
 * A user object that computes the outflow boundary flux
 */
class CNSFVFreeOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  CNSFVFreeOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~CNSFVFreeOutflowBoundaryFlux();

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
};

#endif
