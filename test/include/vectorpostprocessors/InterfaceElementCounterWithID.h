//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceVectorPostprocessor.h"

class InterfaceElementCounterWithID : public InterfaceVectorPostprocessor
{
public:
  static InputParameters validParams();

  InterfaceElementCounterWithID(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// element ID index
  const unsigned int _id_index;

  /// current element ID
  const dof_id_type & _current_id;

  /// current neighbor element ID
  const dof_id_type & _neighbor_id;

  /// unique element IDs on the blocks where this vector pp is defined
  const std::set<dof_id_type> _unique_ids;

  /// element IDs
  VectorPostprocessorValue & _ids;

  /// number of elements
  VectorPostprocessorValue & _nelems;

  /// number of elements on neighbor side
  VectorPostprocessorValue & _neighbor_nelems;

  /// element counters
  std::unordered_map<dof_id_type, std::pair<unsigned int, unsigned int>> _counters;
};
