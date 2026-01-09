//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

#pragma once

#include "ExplicitMixedOrder.h"
#include "NEML2Assembly.h"
#include "NEML2FEInterpolation.h"

class NEML2CentralDifference : public ExplicitMixedOrder
{
public:
  static InputParameters validParams();

  NEML2CentralDifference(const InputParameters & parameters);

  void initialSetup() override;
  void postSolve() override;

protected:
  void evaluateRHSResidual() override;

  /// The assembly object with cached assembly information
  NEML2Assembly * _neml2_assembly = nullptr;

  /// The FE interface for getting variable values/gradients interpolated onto the finite element space
  NEML2FEInterpolation * _fe = nullptr;

private:
  /// Empty element vector to help zero out the algebraic range
  std::vector<const Elem *> _boundary_elements = {};

  /// Empty node vector to help zero out the algebraic range
  std::vector<const Node *> _no_node = {};
};

#endif // NEML2_ENABLED
