//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Base class for heat source components
 */
class HeatSourceBase : public Component
{
public:
  HeatSourceBase(const InputParameters & parameters);

protected:
  virtual void check() const override;

  /// Heat structure name
  const std::string & _hs_name;

  /// Names of the heat structure regions where heat generation is to be applied
  const std::vector<std::string> & _region_names;

  /// Names of the heat structure subdomains corresponding to the given regions
  std::vector<SubdomainName> _subdomain_names;

public:
  static InputParameters validParams();
};
