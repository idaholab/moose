//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"
#include "ADMaterial.h"

class CNSKernel;

declareADValidParams(CNSKernel);

/**
 * Abstract base class for defining all "single" kernels in Pronghorn.
 * This class is used for defining individual physics pieces as well as
 * the "single-equation" form of SUPG stabilization, if turned enabled.
 *
 * The "single-equation" SUPG stabilization refers to stabilization of an
 * individual conservation equation against only itself. The `SUPGKernel`,
 * on the other hand, stabilizes each individual conservation equation both
 * against itself _and_ the other conservation equations. Here, the
 * "single-equation" form of the SUPG stabilization for a kernel with generic
 * strong form $f$ is $\tau f\vec{V}\cdot\nabla\psi$.
 *
 * Using the "single-equation" form of the SUPG stabilization should only be
 * used when SUPG stabilization is applied to a single equation. This is
 * typically used only for the legacy set of equations, and SUPG stabilization
 * is applied in that case to only the fluid energy equation.
 */
class CNSKernel : public ADKernel
{
public:
  CNSKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override final;

  /// compute the weak residual of the kernel
  virtual ADReal weakResidual() = 0;

  /// compute the strong residual of the kernel
  virtual ADReal strongResidual() = 0;

  /// r-z coordinate
  const unsigned int _rz_coord;

  /// whether to turn on the single-equation form of the SUPG stabilization
  const bool & _single_eqn_supg;

  /// SUPG stabilization parameter
  const ADMaterialProperty<RealVectorValue> * _tau;

};
