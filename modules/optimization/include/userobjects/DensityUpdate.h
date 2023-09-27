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
  const MooseMesh & _mesh;
  const VariableName _design_density_name;
  const VariableName _density_sensitivity_name;

  MooseVariable & _design_density;
  const MooseVariable & _density_sensitivity;

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

  void gatherElementData();
  void performOptimCritLoop();
  void computePhysDensity();

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
