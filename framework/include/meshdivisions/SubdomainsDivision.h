//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshDivision.h"
#include "BlockRestrictable.h"

/**
 * Divides the mesh based on the subdomains
 */
class SubdomainsDivision : public MeshDivision, public BlockRestrictable
{
public:
  static InputParameters validParams();

  SubdomainsDivision(const InputParameters & parameters);

  virtual void initialize() override;
  virtual unsigned int divisionIndex(const Point & pt) const override;
  virtual unsigned int divisionIndex(const Elem & elem) const override;

protected:
  /// Map from subdomain ids to division index. Created on calls to initialize()
  std::unordered_map<SubdomainID, unsigned int> _subdomain_ids_to_division_index;
};
