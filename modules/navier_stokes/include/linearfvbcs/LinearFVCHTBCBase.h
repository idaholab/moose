//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Base class that ensures that we can compute the conductive heat flux
 * on a boundary. This interface is mainly used for boundary field management
 * in the segregated solvers for conjugate heat transfer problems where the
 * coupling between the two variables is based on conductive fluxes and
 * boundary temperatures.
 */
class LinearFVCHTBCBase : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVCHTBCBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryConductionFlux() const = 0;
};
