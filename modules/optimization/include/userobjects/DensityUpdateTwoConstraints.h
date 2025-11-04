//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "MooseTypes.h"

/**
 * Element user object that performs SIMP optimization using a bisection algorithm applying a volume
 * constraint and a cost constraint.
 */
class DensityUpdateTwoConstraints : public ElementUserObject
{
public:
  static InputParameters validParams();

  DensityUpdateTwoConstraints(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void timestepSetup() override;
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject &) override{};

protected:
  /// The system mesh
  const MooseMesh & _mesh;
  /// The name of the pseudo-density variable
  const VariableName _design_density_name;
  /// The elasticity compliance sensitivity name
  const VariableName _density_sensitivity_name;
  const VariableName _cost_density_sensitivity_name;
  const VariableName _cost_name;
  /// The pseudo-density variable

  MooseWritableVariable * _design_density;
  /// The filtered density sensitivity variable (elasticity)
  const MooseWritableVariable * _density_sensitivity;
  /// The filtered density sensitivity variable (cost)
  const MooseWritableVariable * _cost_density_sensitivity;
  /// The cost variable
  const MooseVariable & _cost;
  /// The volume fraction to be enforced
  const Real _volume_fraction;
  /// The cost fraction to be enforced
  const Real _cost_fraction;
  // Relative tolerance used to stop the two-variable bisection method.
  const Real _relative_tolerance;
  // Weights to solve a dual physics problem, e.g. a thermomechanical one.
  std::vector<Real> _weight_values;

private:
  struct ElementData
  {
    Real old_density;
    Real sensitivity;
    Real cost_sensitivity;
    Real thermal_sensitivity;
    Real cost;
    Real volume;
    Real new_density;

    ElementData() = default;
    ElementData(
        Real dens, Real sens, Real cost_sens, Real thermal_sens, Real cst, Real vol, Real filt_dens)
      : old_density(dens),
        sensitivity(sens),
        cost_sensitivity(cost_sens),
        thermal_sensitivity(thermal_sens),
        cost(cst),
        volume(vol),
        new_density(filt_dens)
    {
    }
  };

  /**
   * Gathers element date necessary to perform the bisection algorithm for optimization
   */
  void gatherElementData();
  /**
   * Performs the optimality criterion loop (bisection)
   */
  void performOptimCritLoop();

  Real computeUpdatedDensity(Real current_density,
                             Real dc,
                             Real cost_sensitivity,
                             Real thermal_sensitivity,
                             Real cost,
                             Real lmid,
                             Real cmid);

  /// Total volume allowed for volume contraint
  Real _total_allowable_volume;

  /// Total volume allowed for cost contraint
  Real _total_allowable_cost;

  /// Data structure to hold old density, sensitivity, volume, current density.
  std::map<dof_id_type, ElementData> _elem_data_map;

  /// Lower bound for bisection algorithm
  const Real _lower_bound;
  /// Upper bound for bisection algorithm
  const Real _upper_bound;
  /// Bisection algorithm move
  const Real _bisection_move;
  /// Whether bisection moves are adaptive
  const bool _adaptive_move;
  /// Thermal compliance sensitivity name
  VariableName _thermal_sensitivity_name;
  /// Thermal compliance variable
  MooseVariable * _thermal_sensitivity;
};
