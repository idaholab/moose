//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "MooseTypes.h"

class PenetrationLocator;
class NearestNodeLocator;
class SystemBase;
namespace libMesh
{
template <typename>
class NumericVector;
}

/**
 * Creates an AD material property for a variable transferred from the boundary
 * of a 2D mesh onto a 1D mesh.
 */
class VariableValueTransferMaterial : public Material
{
public:
  VariableValueTransferMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeProperties() override;

  /// Penetration locator
  PenetrationLocator & _penetration_locator;
  /// Nearest node locator
  NearestNodeLocator & _nearest_node;
  /// Nonlinear system
  const SystemBase & _nl_sys;
  /// Solution vector
  const NumericVector<Number> * const & _serialized_solution;
  /// Variable number of the variable to transfer
  unsigned int _paired_variable;
  /// Material property for the transferred variable
  ADMaterialProperty<Real> & _prop;
  /// Basis function for transferred variable
  const VariablePhiValue & _phi;
};
