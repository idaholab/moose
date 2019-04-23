//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshModifier.h"

class LowerDBlockFromSideset;

template <>
InputParameters validParams<LowerDBlockFromSideset>();

/**
 * Creates lower-dimensional elements on the specified sidesets
 */
class LowerDBlockFromSideset : public MeshModifier
{
public:
  LowerDBlockFromSideset(const InputParameters & parameters);

protected:
  virtual void modify() override;

  /// The subdomain ID of the new lower dimensional block
  const SubdomainID _new_block_id;
  /// The sidesets on which to create the lower dimensional elements
  std::vector<BoundaryID> _sidesets;
};

