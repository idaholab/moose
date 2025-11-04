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
 * Element user object that performs SIMP optimization using a bisection algorithm using a volume
 * constraint.
 */
class DensityUpdate : public ElementUserObject
{
public:
  static InputParameters validParams();

  DensityUpdate(const InputParameters & parameters);

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
  /// The pseudo-density variable
  MooseWritableVariable * _design_density;
  /// The filtered density sensitivity variable
  const MooseWritableVariable * _density_sensitivity;
  /// The volume fraction to be enforced
  const Real _volume_fraction;

private:
  struct ElementData
  {
    Real old_density;
    Real sensitivity;
    Real volume;
    Real new_density;
    ElementData() = default;
    ElementData(Real dens, Real sens, Real vol, Real filt_dens)
      : old_density(dens), sensitivity(sens), volume(vol), new_density(filt_dens)
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

  Real computeUpdatedDensity(Real current_density, Real dc, Real lmid);

  /// Total volume allowed for volume contraint
  Real _total_allowable_volume;

  /// Data structure to hold old density, sensitivity, volume, current density.
  std::map<dof_id_type, ElementData> _elem_data_map;

  /// Lower bound for bisection algorithm
  const Real _lower_bound;
  /// Upper bound for bisection algorithm
  const Real _upper_bound;
};
