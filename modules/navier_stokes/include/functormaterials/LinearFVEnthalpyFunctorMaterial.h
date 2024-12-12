//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class SinglePhaseFluidProperties;

/**e
 * Converts temperature to enthalpy or enthalpy to temperature using
 * functor material properties. The derivatives are discarded, so can't
 * be used with AD.
 */
class LinearFVEnthalpyFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  LinearFVEnthalpyFunctorMaterial(const InputParameters & parameters);

protected:
  /// Variables, treated as functors, needs to be AD because
  /// Materials inherit from ADFunctorInterface
  const Moose::Functor<ADReal> & _pressure;
  const Moose::Functor<ADReal> & _T_fluid;
  const Moose::Functor<ADReal> & _h;

  /// The fluid properties that containt the h from Tconversion routines
  const SinglePhaseFluidProperties * _fluid;

  /// Pointers to the the conversion functors (in case the fluid property is not provided)
  const Moose::Functor<Real> * _h_from_p_T_functor;
  const Moose::Functor<Real> * _T_from_p_h_functor;

  using UserObjectInterface::getUserObject;
};
