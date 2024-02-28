//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "ADComputeIncrementalShellStrain.h"

/**
 * ADComputeFiniteShellStrain computes the strain increment term for shell elements under finite
 * displacement/rotation scenarios.
 **/

class ADComputeFiniteShellStrain : public ADComputeIncrementalShellStrain
{
public:
  static InputParameters validParams();

  ADComputeFiniteShellStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeProperties() override;
  virtual void computeNodeNormal() override;
  virtual void updatedxyz() override;
  virtual void updateGVectors() override;

  /// Computes the B_nl matrix that connects the nonlinear strains to the nodal displacements and rotations
  virtual void computeBNLMatrix();

  /// Material property to store the B_nl matrix at each quadrature point
  std::vector<ADMaterialProperty<DenseMatrix<Real>> *> _B_nl;
};
