//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrackFrontNonlocalMaterialBase.h"

/**
 * This vectorpostprocessor computes the average scalar material normal to the crack front at points
 * along the crack front.
 */
class CrackFrontNonlocalScalar : public CrackFrontNonlocalMaterialBase
{
public:
  static InputParameters validParams();

  CrackFrontNonlocalScalar(const InputParameters & parameters);

protected:
  /// The scalar material property
  const MaterialProperty<Real> & _scalar;

  Real getQPCrackFrontScalar(const unsigned int qp, Point /*direction*/) const override;
};
