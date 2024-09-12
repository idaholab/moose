//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifierBase.h"

class ElementSubdomainModifier : public ElementSubdomainModifierBase
{
public:
  static InputParameters validParams();

  ElementSubdomainModifier(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override;
  virtual void finalize() override;

protected:
  /**
   * Compute the subdomain ID of the current element
   *
   * If the computed subdomain ID is different from the current element's current subdomain ID, the
   * current element WILL be "moved" from its current subdomain to the computed subdomain.
   *
   * If the computed subdomain ID is (1) the same as the current element's current subdomain ID or
   * (2) Moose::INVALID_BLOCK_ID, the current element does not participate in subdomain modification
   * activities.
   */
  virtual SubdomainID computeSubdomainID() = 0;

  /// Element subdomain assignments
  std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> _moved_elems;
};
