//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * Test object to verify that the map built by MooseMesh::buildPeriodicNodeMap() contains all
 * necessary entries. In particular this object is designed to verify corner cases (e.g. in 2D
 * with both directions being assigned periodicity this object verifies that each corner maps to
 * two other corners for a total of 8 corner entries, there are 27 in 3D). This object doesn't
 * return anything but produces errors for missing entries and warnings for corner nodes with fewer
 * than expected pairings.
 */
class PeriodicNodeMapTester : public ElementUserObject
{
public:
  static InputParameters validParams();

  PeriodicNodeMapTester(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual void threadJoin(const UserObject &) override{};

protected:
  /// Coupled variable id
  const unsigned int _v_var;

  /// mesh dimension
  const unsigned int _dim;

  ///@{ periodic size per component
  std::array<Real, LIBMESH_DIM> _periodic_min;
  std::array<Real, LIBMESH_DIM> _periodic_max;
  ///@}

  /// We time the periodic node list build ourselves to ste the level to 1
  const PerfID _perf_buildmap;
};
