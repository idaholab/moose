//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CoupledForce.h"

template <bool is_ad>
class RenamedCoupledForceTempl : public CoupledForceTempl<is_ad>
{
public:
  static InputParameters validParams();

  RenamedCoupledForceTempl(const InputParameters & parameters);
};

typedef RenamedCoupledForceTempl<false> RenamedCoupledForce;
typedef RenamedCoupledForceTempl<true> ADRenamedCoupledForce;
