//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorIntegratedBC.h"

/**
 * A class that imparts a surface tension on the momentum equation
 * The treatment is based on:
 * Cairncross RA, Schunk PR, Baer TA, Rao RR, Sackinger PA. A finite element method for free surface
 * flows of incompressible fluids in three dimensions. Part I. Boundary fitted mesh motion.
 * International journal for numerical methods in fluids. 2000 Jun 15;33(3):375-403.
 */
class INSADSurfaceTensionBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  INSADSurfaceTensionBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The surface tension terms
  const ADMaterialProperty<RealVectorValue> & _surface_term_curvature;
  const ADMaterialProperty<RealVectorValue> & _surface_term_gradient1;
  const ADMaterialProperty<RealVectorValue> & _surface_term_gradient2;

private:
  /// If the surface tension should include the gradient terms
  /// (increases fidelity, decreases stability)
  const bool _include_gradient_terms;

  /// Curvature force multiplier. The reason behind this is that
  /// libmesh has a different sign convention for 2D and 3D.
  const Real _curvature_factor;
};
