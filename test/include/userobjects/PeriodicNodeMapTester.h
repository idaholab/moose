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

class PeriodicNodeMapTester;

template <>
InputParameters validParams<PeriodicNodeMapTester>();

/**
 * Test MooseMesh::buildPeriodicNodeMap()
 */
class PeriodicNodeMapTester : public ElementUserObject
{
public:
  PeriodicNodeMapTester(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual void threadJoin(const UserObject &) override{};

protected:
  /// Coupled variable id
  unsigned int _v_var;

  /// mesh dimension
  unsigned int _dim;

  ///@{ periodic size per component
  std::array<Real, LIBMESH_DIM> _periodic_min;
  std::array<Real, LIBMESH_DIM> _periodic_max;
  ///@}

  /// We time the periodic node list build ourselves to ste the level to 1
  PerfID _perf_buildmap;
};

