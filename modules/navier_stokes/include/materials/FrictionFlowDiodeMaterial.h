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

/**
 * Linear friction term when velocity is opposite the direction of the diode
 */
class FrictionFlowDiodeMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FrictionFlowDiodeMaterial(const InputParameters & parameters);

private:
  /// Direction of the diode
  const RealVectorValue _direction;

  /// A linear friction coefficient applied when velocity is opposite the direction
  const Real _resistance;

  /// Base linear friction coefficient, from the correlation for the porous media friction
  const Moose::Functor<ADRealVectorValue> & _base_friction;

  ///@{Components of the superficial velocity
  const Moose::Functor<ADReal> & _u;
  const Moose::Functor<ADReal> & _v;
  const Moose::Functor<ADReal> & _w;
  ///@}
};
