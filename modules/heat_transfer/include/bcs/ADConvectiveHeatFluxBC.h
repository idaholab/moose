//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Boundary condition for convective heat flux where temperature and heat transfer coefficient are
 * given by material properties.
 */
class ADConvectiveHeatFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  ADConvectiveHeatFluxBC(const InputParameters & parameters);

  /**
   * Here we check if the functors are defined on primary side of the boundary. If they are not, the
   * neighboring elements are used, if available. Errors are raised if the functors are not defined
   * on either all of the primary side or all of the neighboring side.
   */
  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual() override;

  /// Far-field temperature variable
  const ADMaterialProperty<Real> * const _T_infinity;

  /// Convective heat transfer coefficient
  const ADMaterialProperty<Real> * const _htc;

  /// Far-field temperature functor
  const Moose::Functor<ADReal> * const _T_infinity_functor;

  /// Convective heat transfer coefficient as a functor
  const Moose::Functor<ADReal> * const _htc_functor;

  /// Whether the far-field temperature functor should be evaluated on neighbor elements
  bool _T_infinity_use_neighbor = false;

  /// Whether the heat transfer coefficient functor should be evaluated on neighbor elements
  bool _htc_use_neighbor = false;

  /// Neighbor of the current element's side (can be nullptr)
  const Elem * _current_neighbor_elem = nullptr;

  /// Corresponding side on the neighbor
  unsigned int _current_neighbor_side = libMesh::invalid_uint;
};
