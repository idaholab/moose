//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_LIBTORCH_ENABLED

#include "ExplicitMixedOrder.h"
#include "TorchAssembly.h"
#include "TorchFEInterpolation.h"

class TorchCentralDifference : public ExplicitMixedOrder
{
public:
  static InputParameters validParams();

  TorchCentralDifference(const InputParameters & parameters);

  void initialSetup() override;
  void meshChanged() override;
  void postSolve() override;

protected:
  void evaluateRHSResidual() override;
  void rebuildBoundaryElementList();

  /// The assembly object with cached assembly information
  TorchAssembly * _assembly = nullptr;

  /// The FE interface for getting variable values/gradients interpolated onto the finite element
  /// space
  TorchFEInterpolation * _fe = nullptr;

private:
  /// Empty element vector to help zero out the algebraic range
  std::vector<const Elem *> _boundary_elems = {};

  /// Empty node vector to help zero out the algebraic range
  std::vector<const Node *> _no_node = {};

  /// Whether the cached boundary element list needs rebuilding
  bool _boundary_elems_dirty = true;
};

#endif // MOOSE_LIBTORCH_ENABLED
