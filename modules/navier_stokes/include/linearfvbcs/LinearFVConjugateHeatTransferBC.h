//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVConvectiveHeatTransferBC.h"

/**
 * Class describing a convective heat transfer between two domains.
 * The heat flux is described  by:
 * h * (T_solid - T_liquid),
 * where h is the heat transfer coefficient, while T_solid and T_liquid
 * denote the solid and liquid temperatures, respectively.
 */
class LinearFVConjugateHeatTransferBC : public LinearFVConvectiveHeatTransferBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVConjugateHeatTransferBC(const InputParameters & parameters);

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:

  const Moose::Functor<Real> & _solid_conductivity;

  const Moose::Functor<Real> & _fluid_conductivity;

  const Moose::Functor<Real> * _rhs_conductivity;
};
