//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * This BC applies a heat flux to a boundary, where the heat flux is
 * determined using series conduction resistances, and parallel convection
 * and radiation resistances at the surface. It is assumed that this BC is
 * applied to an equation where `_u` represents a temperature. Note that this
 * BC is only valid for pseudo steady-state problems and with no heat sources in
 * the conduction layers. It is also assumed that the surface is isothermal - if
 * this is not the case, this BC essentially assumes parallel heat transfer
 * between each quadrature point and the surface, with no interaction between
 * adjacent quadrature points. This will underpredict the heat transfer if the
 * surface is not isothermal.
 *
 * When a radiation resistance at the surface is included, the solution requires
 * an iterative procedure because the effective heat transfer coefficient for
 * radiative heat transfer depends on the surface temperature. This iteration is
 * performed with a simple underrelaxation method.
 */
class FVThermalResistanceBC : public FVFluxBC
{
public:
  FVThermalResistanceBC(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Computes the parallel heat flux resistance for a combined radiation-convection boundary
  void computeParallelResistance();

  /// Computes the serial resistance of multiple conductive layers
  void computeConductionResistance();

  /// Whether to use a `cylindrical` or `cartesian` form for the thermal resistances.
  /// When using a cylindrical geometry, the inner-most radius must be provided.
  const Moose::CoordinateSystemType _geometry;

  /// Radius corresponding to the cylindrical surface (when using a cylindrical geometry)
  const Real _inner_radius;

  /// temperature variable
  const ADVariableValue & _T;

  /// ambient temperature for convection and radiation heat transfer
  const Real _T_ambient;

  /// thermal conductivities for each conduction layer, listed in order closest to
  /// the boundary
  const std::vector<Real> & _k;

  /// thicknesses for each conduction layer, listed in order closest to the boundary
  const std::vector<Real> & _dx;

  /// convective heat transfer coefficient
  const ADMaterialProperty<Real> & _h;

  /// boundary emissivity
  const Real & _emissivity;

  /// maximum number of iterations (when radiative heat transfer is included)
  const unsigned int & _max_iterations;

  /// tolerance of iterations (when radiative heat transfer is included)
  const Real & _tolerance;

  /// underrelaxation factor (when radiative heat transfer is included)
  const Real & _alpha;

  /// surface temperature
  ADReal _T_surface;

  /// outer radius of surface
  ADReal _outer_radius;

  /// conduction thermal resistance
  ADReal _conduction_resistance;

  /// parallel convection and radiation thermal resistance
  ADReal _parallel_resistance;
};
