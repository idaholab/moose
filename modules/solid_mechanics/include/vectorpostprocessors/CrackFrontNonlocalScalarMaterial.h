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
 * Computes the average material at points provided by the crack_front_definition
 * vectorpostprocessor.
 */
class CrackFrontNonlocalScalarMaterial : public CrackFrontNonlocalMaterialBase
{
public:
  static InputParameters validParams();

  CrackFrontNonlocalScalarMaterial(const InputParameters & parameters);

protected:
  /// Property that is averaged over the crack front points
  const MaterialProperty<Real> & _scalar;

  Real getQPCrackFrontScalar(const unsigned int qp,
                             const Point /*crack_face_normal*/) const override;
};
