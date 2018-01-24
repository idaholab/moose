//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MESHSIDESET_H
#define MESHSIDESET_H

#include "MeshModifier.h"

class MeshSideSet;

template <>
InputParameters validParams<MeshSideSet>();

/**
 * Add lower dimensional elements along the faces contained in a side set
 */
class MeshSideSet : public MeshModifier
{
public:
  MeshSideSet(const InputParameters & parameters);

  virtual void modify() override;

protected:
  /// Block ID to assign to the region
  const SubdomainID _block_id;
};

#endif // MESHSIDESET_H
