//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoFieldScalarHDGBC.h"

/**
 * Implements a prescribed flux for an IP-HDG discretization
 */
class IPHDGPrescribedFluxBC : public TwoFieldScalarHDGBC
{
public:
  static InputParameters validParams();

  IPHDGPrescribedFluxBC(const InputParameters & parameters);

private:
  /**
   * compute the AD residuals
   */
  virtual void compute(TwoFieldScalarHDGAssemblyHelper & helper) override;

  /// Prescribed normal flux along the boundary. The default is 0 for a natural boundary
  /// condition
  const Moose::Functor<Real> & _normal_flux;
};
