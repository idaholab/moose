//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVConjugateHeatTransferBCBase.h"

class LinearFVRobinConjugateHeatTransferBC : public LinearFVConjugateHeatTransferBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVRobinConjugateHeatTransferBC(const InputParameters & parameters);

  static InputParameters validParams();

  // virtual void initialSetup() override;

  virtual Real computeBoundaryConductionFlux() const override;

protected:
  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

protected:
  /// The convective heat transfer coefficient
  const Moose::Functor<Real> & _htc;

  const Moose::Functor<Real> * _incoming_flux;

  const Moose::Functor<Real> * _incoming_temperature;
};
