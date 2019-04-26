//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BreakMeshByBlockBase.h"

// forward declaration
class BreakMeshByBlockManualBase;

template <>
InputParameters validParams<BreakMeshByBlockManualBase>();

class BreakMeshByBlockManualBase : public BreakMeshByBlockBase
{
public:
  BreakMeshByBlockManualBase(const InputParameters & parameters);

  virtual void modify() override; // method to override to implement other mesh splitting algorithms

protected:
  /// method that given an element id and a local node ID duplicate it and set it
  virtual void duplicateAndSetLocalNode(dof_id_type element_id, dof_id_type local_node);

  /// method setting the local_node of element_id to global_node
  virtual void setElemNode(dof_id_type element_id, dof_id_type local_node, dof_id_type global_node);
};

