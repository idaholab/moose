/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AEFVUpwindINTERNALSIDEFLUX_H
#define AEFVUpwindINTERNALSIDEFLUX_H

#include "InternalSideFluxBase.h"

// Forward Declarations
class AEFVUpwindInternalSideFlux;

template <>
InputParameters validParams<AEFVUpwindInternalSideFlux>();

/**
 * Upwind numerical flux scheme
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVUpwindInternalSideFlux : public InternalSideFluxBase
{
public:
  AEFVUpwindInternalSideFlux(const InputParameters & parameters);
  virtual ~AEFVUpwindInternalSideFlux();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const override;

protected:
};

#endif
