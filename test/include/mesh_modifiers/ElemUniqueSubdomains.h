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
#include "MeshModifier.h"

// Forward declerations
class ElemUniqueSubdomains;

template <>
InputParameters validParams<ElemUniqueSubdomains>();

/**
 * MeshModifier for assigning subdomain IDs of all elements
 */
class ElemUniqueSubdomains : public MeshModifier
{
public:
  ElemUniqueSubdomains(const InputParameters & parameters);

protected:
  virtual void modify() override;
};

