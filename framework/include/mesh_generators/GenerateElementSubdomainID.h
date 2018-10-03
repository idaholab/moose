//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERATEELEMENTSUBDOMAINID_H
#define GENERATEELEMENTSUBDOMAINID_H

#include "MeshGenerator.h"

// Forward declarations
class GenerateElementSubdomainID;

template <>
InputParameters validParams<GenerateElementSubdomainID>();

/**
 * MeshGenerator for assigning subdomain IDs of all elements
 */
class GenerateElementSubdomainID : public MeshGenerator
{
public:
  GenerateElementSubdomainID(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;
};

#endif // GENERATEELEMENTSUBDOMAINID_H
