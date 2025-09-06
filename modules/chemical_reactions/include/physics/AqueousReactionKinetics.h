//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReactionKineticsPhysicsBase.h"

/**
 * Creates all the objects needed to solve a reaction network of chemical reactions in an aqueous
 * medium with a continuous Galerkin finite element discretization
 */
class AqueousReactionKinetics : public ReactionKineticsPhysicsBase
{
public:
  static InputParameters validParams();

  AqueousReactionKinetics(const InputParameters & parameters);

protected:
  virtual void addAuxiliaryVariables() override;
  virtual void addFEKernels() override;

  /// Name of the pressure variable
  const std::vector<VariableName> & _pressure_var;
  /// Gravity vector
  const RealVectorValue _gravity;

  /// Vector of vectors, indexed by (i, j), of whether primary solver species 'i' is present in reaction 'j'
  std::vector<std::vector<bool>> _primary_participation;
};
