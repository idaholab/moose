//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"

/**
 * This user object modifies the element subdomain ID based on the provided variable value.
 */
class VariableValueElementSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  VariableValueElementSubdomainModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;

private:
  const VariableValue & _v;
  // save the subdomain IDs that are requested but do not actually exist in the mesh
  std::unordered_set<SubdomainID> _void_sids;
  std::mutex _void_sids_mutex;
};
