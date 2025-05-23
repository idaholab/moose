//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVRadiativeHeatFluxBCBase.h"

/**
 * Boundary condition for radiative heat flux where temperature and the
 * temperature of a body in radiative heat transfer are specified and the emissivity is specified by
 * a user-provided functor.
 */
class FVFunctorRadiativeBC : public FVRadiativeHeatFluxBCBase
{
public:
  static InputParameters validParams();

  FVFunctorRadiativeBC(const InputParameters & parameters);

protected:
  virtual Real coefficient() const override;

  /// emissivity
  const Moose::Functor<Real> & _emissivity;
};
