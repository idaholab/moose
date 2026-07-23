//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMElementSubdomainModifierBase.h"
#include "PointInSubdomainCheckUO.h" // UO with subdomain geometry checks
#include "Function.h"

class SubdomainElementModifier : public SBMElementSubdomainModifierBase
{
public:
  static InputParameters validParams();
  SubdomainElementModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;

  /// The user object containing subdomain-wise in/out checkers
  const PointInSubdomainCheckUO & _subdomain_id_tester;
};
