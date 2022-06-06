//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1DConnection.h"

/**
 * Base class for junctions of 1D components
 */
class Component1DJunction : public Component1DConnection
{
public:
  Component1DJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// Element IDs of connected flow channels
  std::vector<dof_id_type> _connected_elems;
  /// Processor IDs owning the connected flow channels
  std::vector<processor_id_type> _proc_ids;

public:
  static InputParameters validParams();
};
