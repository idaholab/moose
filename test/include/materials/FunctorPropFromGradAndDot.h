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

class FunctorPropFromGradAndDot : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FunctorPropFromGradAndDot(const InputParameters & parameters);

private:
  const Moose::Functor<ADReal> & _functor;
};
