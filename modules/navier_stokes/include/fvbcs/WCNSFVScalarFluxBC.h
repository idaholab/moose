//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVFluxBCBase.h"

/**
 * Flux boundary condition for the weakly compressible scalar advection equation
 */
class WCNSFVScalarFluxBC : public WCNSFVFluxBCBase
{
public:
  static InputParameters validParams();
  WCNSFVScalarFluxBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// override because energy_pp is not considered in base class
  virtual bool isInflow() const override;

  /// Postprocessor with the inlet scalar concentration
  const PostprocessorValue * const _scalar_value_pp;

  /// Postprocessor with the inlet scalar flow rate
  const PostprocessorValue * const _scalar_flux_pp;

  /// passive scalar functor
  const Moose::Functor<ADReal> & _passive_scalar;
};
