//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */
class GapConductance : public Material
{
public:
  enum GAP_GEOMETRY
  {
    PLATE,
    CYLINDER,
    SPHERE
  };
  static InputParameters validParams();

  GapConductance(const InputParameters & parameters);

  static InputParameters actionParameters();

  static Real gapLength(const GAP_GEOMETRY & gap_geom,
                        const Real radius,
                        const Real r1,
                        const Real r2,
                        const Real max_gap);

  /**
   * Compute gap distance for plate geometry
   */
  static Real gapRect(const Real distance, const Real max_gap);
  /**
   * Compute gap distance for cylinder geometry
   */
  static Real gapCyl(const Real radius, const Real r1, const Real r2, const Real max_denom);
  /**
   * Compute gap distance for sphere geometry
   */
  static Real gapSphere(const Real radius, const Real r1, const Real r2, const Real max_denom);

  static Real gapAttenuation(Real adjusted_length, Real min_gap, unsigned int min_gap_order);

  static void setGapGeometryParameters(const InputParameters & params,
                                       const Moose::CoordinateSystemType coord_sys,
                                       unsigned int axisymmetric_radial_coord,
                                       GAP_GEOMETRY & gap_geometry_type,
                                       Point & p1,
                                       Point & p2);

  /// Compute current gap radii for surface integration of gas conductance
  static void computeGapRadii(const GAP_GEOMETRY gap_geometry_type,
                              const Point & current_point,
                              const Point & p1,
                              const Point & p2,
                              const Real & gap_distance,
                              const Point & current_normal,
                              Real & r1,
                              Real & r2,
                              Real & radius);

  virtual void initialSetup() override;

  /// Legacy method that clamps at min_gap
  static Real gapLength(
      const GAP_GEOMETRY & gap_geom, Real radius, Real r1, Real r2, Real min_gap, Real max_gap)
  {
    return std::max(min_gap, gapLength(gap_geom, radius, r1, r2, max_gap));
  }

protected:
  virtual void computeQpProperties() override;

  /**
   * Override this to compute the conductance at _qp
   */
  virtual void computeQpConductance();

  virtual Real h_conduction();
  virtual Real h_radiation();
  virtual Real dh_conduction();
  virtual Real dh_radiation();
  virtual Real gapK();

  virtual void computeGapValues();

  const std::string _appended_property_name;

  const VariableValue & _temp;

  GAP_GEOMETRY & _gap_geometry_type;

  bool _quadrature;

  Real _gap_temp;
  Real _gap_distance;
  Real _radius;
  Real _r1;
  Real _r2;

  bool _has_info;

  const VariableValue & _gap_distance_value;
  const VariableValue & _gap_temp_value;
  MaterialProperty<Real> & _gap_conductance;
  MaterialProperty<Real> & _gap_conductance_dT;
  MaterialProperty<Real> & _gap_thermal_conductivity;

  const Real _gap_conductivity;
  const Function * const _gap_conductivity_function;
  const VariableValue * const _gap_conductivity_function_variable;

  const Real _stefan_boltzmann;
  Real _emissivity;

  const Real _min_gap;
  const unsigned int _min_gap_order;
  const Real _max_gap;

  MooseVariable * _temp_var;
  PenetrationLocator * _penetration_locator;
  const NumericVector<Number> * const * _serialized_solution;
  DofMap * _dof_map;
  const bool _warnings;

  Point & _p1;
  Point & _p2;
};
