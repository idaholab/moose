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

class LinearFVConjugateHeatTransferBCBase : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVConjugateHeatTransferBCBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryConductionFlux() const;

protected:
  const Moose::Functor<Real> & _thermal_conductivity;
};
