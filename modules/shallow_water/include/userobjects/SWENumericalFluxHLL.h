//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SWENumericalFluxBase.h"

/**
 * Simple HLL/Rusanov-style numerical flux for 2D SWE.
 *
 * Stub implementation: provides consistent flux; Jacobians are zero (placeholder).
 */
class SWENumericalFluxHLL : public SWENumericalFluxBase
{
public:
  static InputParameters validParams();

  SWENumericalFluxHLL(const InputParameters & parameters);
  virtual ~SWENumericalFluxHLL();

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
};
