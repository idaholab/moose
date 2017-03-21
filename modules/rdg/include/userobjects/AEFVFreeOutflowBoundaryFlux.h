/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AEFVFREEOUTFLOWBOUNDARYFLUX_H
#define AEFVFREEOUTFLOWBOUNDARYFLUX_H

#include "BoundaryFluxBase.h"

// Forward Declarations
class AEFVFreeOutflowBoundaryFlux;

template <>
InputParameters validParams<AEFVFreeOutflowBoundaryFlux>();

/**
 * Free outflow BC based boundary flux user object
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVFreeOutflowBoundaryFlux : public BoundaryFluxBase
{
public:
  AEFVFreeOutflowBoundaryFlux(const InputParameters & parameters);
  virtual ~AEFVFreeOutflowBoundaryFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        const std::vector<Real> & uvec1,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<Real> & uvec1,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1) const override;

protected:
};

#endif
