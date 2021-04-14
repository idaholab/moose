//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVHLLCSourceBC.h"

class NSFVHLLCReactionBC : public NSFVHLLCSourceBC
{
public:
  NSFVHLLCReactionBC(const InputParameters & params);

  static InputParameters validParams();

protected:
  // ADReal computeQpResidual() override;
  ADReal sourceElem() override;
  const unsigned int _component;
  const Real _rate;
};
