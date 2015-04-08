/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAPCONDUCTANCE_H
#define GAPCONDUCTANCE_H

#include "Material.h"

/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */
class GapConductance :
  public Material
{
public:

  GapConductance(const std::string & name, InputParameters parameters);

  virtual ~GapConductance(){}

  static Real gapLength(const Moose::CoordinateSystemType & system, Real radius, Real r1, Real r2, Real min_gap, Real max_gap);

  static Real gapRect(Real distance, Real min_gap, Real max_gap);

  static Real gapCyl( Real radius, Real r1, Real r2, Real min_denom, Real max_denom);

  static Real gapSphere( Real radius, Real r1, Real r2, Real min_denom, Real max_denom);

protected:

  virtual void computeProperties();
  virtual void computeQpProperties();

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

  bool _gap_type_set;
  Moose::CoordinateSystemType _gap_type;

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

  const Real _gap_conductivity;
  Function * const _gap_conductivity_function;
  const VariableValue * _gap_conductivity_function_variable;

  const Real _stefan_boltzmann;
  Real _emissivity;

  Real _min_gap;
  Real _max_gap;

  MooseVariable * _temp_var;
  PenetrationLocator * _penetration_locator;
  const NumericVector<Number> * * _serialized_solution;
  DofMap * _dof_map;
  const bool _warnings;
};

template<>
InputParameters validParams<GapConductance>();

#endif //GAPCONDUCTANCE_H
