//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericConstantMaterial.h"

class ActiveGenericConstantMaterial : public GenericConstantMaterial
{
public:
  static InputParameters validParams();

  ActiveGenericConstantMaterial(const InputParameters & parameters);

  const std::unordered_set<unsigned int> & getActivePropIDs() { return _active_prop_ids; }

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;
};
