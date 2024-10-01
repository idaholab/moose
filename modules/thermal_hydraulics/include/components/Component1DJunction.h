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

  /// Gets the element IDs of the connected 1D components
  const std::vector<dof_id_type> & getConnectedElementIDs() { return _connected_elems; }
  /// Gets the processor IDs of the connected 1D components
  const std::vector<processor_id_type> & getConnectedProcessorIDs() { return _proc_ids; }

  // TODO: make _connected_elems and _proc_ids private (after applications
  // switch to using the getter methods)
  /// Element IDs of connected 1D components
  std::vector<dof_id_type> _connected_elems;
  /// Processor IDs of connected 1D components
  std::vector<processor_id_type> _proc_ids;

  /// Junction subdomain ID
  subdomain_id_type _junction_subdomain_id;

public:
  static InputParameters validParams();
};
