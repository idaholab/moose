//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * ErrorMaterial calls a mooseError at the locations desired
 * met.
 */
class ErrorMaterial : public Material
{
public:
  static InputParameters validParams();

  ErrorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The MaterialProperty value we are responsible for computing
  MaterialProperty<Real> & _prop_value;
  /// The list of boundary ids for the boundaries to error on
  std::vector<BoundaryID> _boundary_ids_to_error;
};
