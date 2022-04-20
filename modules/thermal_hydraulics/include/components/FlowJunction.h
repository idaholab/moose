//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowConnection.h"

/**
 * Base class for flow junctions
 */
class FlowJunction : public FlowConnection
{
public:
  FlowJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;
  virtual void initSecondary() override;

  /// Name of junction user object name, if any
  const std::string _junction_uo_name;
  /// Element IDs of connected flow channels
  std::vector<dof_id_type> _connected_elems;
  /// Processor IDs owning the connected flow channels
  std::vector<processor_id_type> _proc_ids;

public:
  static InputParameters validParams();
};
