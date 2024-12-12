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
#include "Function.h"

class SinglePhaseFluidProperties;
class Function;

/**e
 * Computes fluid properties in (P, T) formulation using functor material properties
 */
class LinearFVEnthalpyFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  LinearFVEnthalpyFunctorMaterial(const InputParameters & parameters);

protected:
  /// variables
  const Moose::Functor<ADReal> & _pressure;
  const Moose::Functor<ADReal> & _T_fluid;
  const Moose::Functor<ADReal> & _h;

  const SinglePhaseFluidProperties * _fluid;
  const Moose::Functor<Real> * _h_from_p_T_functor;
  const Moose::Functor<Real> * _T_from_p_h_functor;

  using UserObjectInterface::getUserObject;
};
