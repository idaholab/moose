//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

class GapFluxModelBase;

/**
 * This Constraint implements thermal contact using a "gap
 * conductance" model in which the flux is represented by an
 * independent "Lagrange multiplier" like variable.
 */
class ModularGapConductanceConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ModularGapConductanceConstraint(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  /**
   * Computes the residual for the LM equation, lambda = (k/l)*(T^(1) - PT^(2)).
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) override;

  /// Gap flux model names
  std::vector<UserObjectName> _gap_flux_model_names;

  /// Gap flux models
  std::vector<const GapFluxModelBase *> _gap_flux_models;

  ///@{ Displacement variables
  const std::vector<std::string> _disp_name;
  const unsigned int _n_disp;
  std::vector<const ADVariableValue *> _disp_secondary;
  std::vector<const ADVariableValue *> _disp_primary;
  ///@}

private:
  virtual ADReal computeSurfaceIntegrationFactor() const;

  virtual ADReal computeGapLength() const;

  /**
   * Computes radii as a function of point and geometry
   */
  void computeGapRadii(const ADReal & gap_length);

  /// Gap geometry (user input or internally overwritten)
  enum class GapGeometry
  {
    AUTO,
    PLATE,
    CYLINDER,
    SPHERE,
  } _gap_geometry_type;

  virtual void setGapGeometryParameters(const InputParameters & params,
                                        const Moose::CoordinateSystemType coord_sys,
                                        unsigned int axisymmetric_radial_coord,
                                        GapGeometry & gap_geometry_type);

  /// Automatically set up axis/center for 2D cartesian problems with cylindrical/spherical gap geometry
  void deduceGeometryParameters();

  /// Gap width to pass into flux models
  ADReal _gap_width;

  /// Factor to preserve energy balance (due to mismatch in primary/secondary differential surface sizes)
  ADReal _surface_integration_factor;

  ///@{ Points for geometric definitions
  Point & _p1;
  Point & _p2;
  ///@}

  ///@{ Radii for quadrature points
  ADReal _r1;
  ADReal _r2;
  ADReal _radius;
  ///@}

  const Real _max_gap;

  ADReal _adjusted_length;

  /// x-displacement variable
  const MooseVariable * const _disp_x_var;
  /// y-displacement variable
  const MooseVariable * const _disp_y_var;
  /// z-displacement variable
  const MooseVariable * const _disp_z_var;

  /// Cached contact pressure for use by UserObjects
  ADReal _normal_pressure;

  friend class GapFluxModelBase;
};
