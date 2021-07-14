//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Computes an implicit boundary flux for the term \f$n_i \epsilon p\f$ where \f$i\f$ denotes the
 * conservation of momentum component equation that this object is acting on (represented by \p
 * _index). This object is useful for wall boundaries where the momentum advective flux is zero and
 * consequently the pressure term is not included in other \p FVFluxBCs
 */
class PCNSFVImplicitMomentumPressureBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  PCNSFVImplicitMomentumPressureBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _eps;
  const unsigned int _index;
};
