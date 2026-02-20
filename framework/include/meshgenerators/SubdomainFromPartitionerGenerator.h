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
#include "MooseEnum.h"

/**
 * MeshGenerator for defining subdomains from the output of the partitioner
 */
class SubdomainFromPartitionerGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SubdomainFromPartitionerGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Offset to start numbering subdomains with
  const subdomain_id_type _offset;
};
