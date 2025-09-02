//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

class SubdomainExtraElementIDGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  SubdomainExtraElementIDGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// input mesh for adding element IDs
  std::unique_ptr<MeshBase> & _input;
  /// subdomains that are to be assigned with element IDs
  const std::vector<SubdomainName> & _subdomain_names;
  /// The names for each ID from input
  const std::vector<std::string> & _id_names;
  /// The IDs from input
  const std::vector<std::vector<dof_id_type>> & _ids;
  /// The default IDs from input, if any
  const std::vector<dof_id_type> * const _defaults;
};
