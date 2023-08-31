//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "MooseTypes.h"

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
  const MooseMesh & _mesh;
  const VariableName _design_density_name;
  const VariableName _density_sensitivity_name;
  const VariableName _cost_density_sensitivity_name;
  const VariableName _cost_name;

  MooseVariable & _design_density;
  const MooseVariable & _density_sensitivity;
  const MooseVariable & _cost_density_sensitivity;
  const MooseVariable & _cost;

  const Real _volume_fraction;
  const Real _cost_fraction;
  // Relative tolerance used to stop the two-variable bisection method.
  const Real _relative_tolerance;

private:
  struct ElementData
  {
    Real old_density;
    Real sensitivity;
    Real cost_sensitivity;
    Real cost;
    Real volume;
    Real new_density;

    ElementData() = default;
    ElementData(Real dens, Real sens, Real cost_sens, Real cst, Real vol, Real filt_dens)
      : old_density(dens),
        sensitivity(sens),
        cost_sensitivity(cost_sens),
        cost(cst),
        volume(vol),
        new_density(filt_dens)
    {
    }
  };

  void gatherElementData();
  void performOptimCritLoop();
  void computePhysDensity();

  Real computeUpdatedDensity(Real current_density,
                             Real dc,
                             Real cost_sensitivity,
                             Real cost,
                             Real lmid,
                             Real cmid,
                             Real loop_number);

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
};
