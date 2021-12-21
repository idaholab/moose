//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes the frictional pressure vector for three-dimensional mortar mechanical contact.
 */
class MortarNodalGapAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MortarNodalGapAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Tangent along the first direction
  const MooseArray<Real> * const _tangent_one;

  /// Tangent along the second direction
  const MooseArray<Real> * const _tangent_two;

  const FEProblemBase & _fe_problem;

  /// Boundary ID for the primary surface
  const BoundaryID _primary_id;

  /// Boundary ID for the secondary surface
  const BoundaryID _secondary_id;

  /// The component
  const unsigned int _component;

  /// Whether to use displaced mesh (required for this auxiliary kernel)
  const bool _use_displaced_mesh;

  /// Handle to mortar generation object to obtain mortar geometry
  const AutomaticMortarGeneration * _mortar_generation_object;
};
