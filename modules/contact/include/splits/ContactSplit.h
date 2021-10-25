//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Split.h"

/**
 * Split-based preconditioner for contact problems.
 */
class ContactSplit : public Split
{
public:
  static InputParameters validParams();

  ContactSplit(const InputParameters & params);
  virtual void setup(const std::string & prefix = "-") override;

protected:
  const std::vector<std::pair<BoundaryName, BoundaryName>> _contact_pairs;
  std::vector<int> _contact_displaced;
  const std::vector<std::pair<BoundaryName, BoundaryName>> _uncontact_pairs;
  std::vector<int> _uncontact_displaced;
  bool _include_all_contact_nodes;
};
