//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERATESUBDOMAINID_H
#define GENERATESUBDOMAINID_H

#include "MeshGenerator.h"

// Forward declarations
class GenerateSubdomainID;

template <>
InputParameters validParams<GenerateSubdomainID>();

/**
 * MeshGenerator for assigning a subdomain ID to all elements
 */
class GenerateSubdomainID : public MeshGenerator
{
public:
  GenerateSubdomainID(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;

  /// The subdomain ID to assign to every elemennt
  SubdomainID _subdomain_id;
};

#endif // GENERATESUBDOMAINID_H
