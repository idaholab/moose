//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeFiniteStrain.h"

/**
 * ComputeRSphericalFiniteStrain defines a strain increment and a rotation increment
 * for finite strains in 1D spherical symmetry geometries.  The strains in the
 * polar and azimuthal directions are functions of the radial displacement.

 */
class ComputeRSphericalFiniteStrain : public ComputeFiniteStrain
{
public:
  static InputParameters validParams();

  ComputeRSphericalFiniteStrain(const InputParameters & parameters);

  virtual void initialSetup() override;

  /// Computes the current and old deformation gradients with the assumptions for
  /// 1D spherical symmetry geometries: \f$ \epsilon_{\theta} = \epsilon_{\phi} = \frac{u_r}{r} \f$
  virtual void computeProperties() override;

protected:
  virtual void computeQpIncrements(RankTwoTensor & e, RankTwoTensor & r) override;

  /// the old value of the first component of the displacements vector
  const VariableValue & _disp_old_0;
};
