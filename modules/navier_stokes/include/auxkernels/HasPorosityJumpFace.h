//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Shows whether an element has any attached porosity jump faces
 */
class HasPorosityJumpFace : public AuxKernel
{
public:
  static InputParameters validParams();

  HasPorosityJumpFace(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// The porosity
  const Moose::Functor<ADReal> & _eps;
};
