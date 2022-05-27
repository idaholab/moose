//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

// Forward Declarations
class Function;

/**
 * GrayLambertSurfaceRadiationBase computes the heat flux on a set of surfaces
 * in radiative heat transfer with each other.
 */
class GrayLambertSurfaceRadiationBase : public SideUserObject
{
public:
  static InputParameters validParams();

  GrayLambertSurfaceRadiationBase(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  bool checkVariableBoundaryIntegrity() const override { return false; }

  /// Define enum for boundary type
  enum RAD_BND_TYPE
  {
    VARIABLE_TEMPERATURE = 0,
    FIXED_TEMPERATURE = 4,
    ADIABATIC = 8
  };

  ///@{ public interface of this UserObject
  Real getSurfaceIrradiation(BoundaryID id) const;
  Real getSurfaceHeatFluxDensity(BoundaryID id) const;
  Real getSurfaceTemperature(BoundaryID id) const;
  Real getSurfaceRadiosity(BoundaryID id) const;
  Real getSurfaceEmissivity(BoundaryID id) const;
  Real getViewFactor(BoundaryID from_id, BoundaryID to_id) const;
  std::set<BoundaryID> getSurfaceIDs() const;
  ///@}

protected:
  virtual void threadJoin(const UserObject & y) override;

  /// a purely virtual function that defines where view factors come from
  virtual std::vector<std::vector<Real>> setViewFactors() = 0;

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// number of active boundary ids
  unsigned int _n_sides;

  /// the coupled temperature variable
  const VariableValue & _temperature;

  /// constant emissivity for each boundary
  const std::vector<Real> _emissivity;

  /// side id to index map, side ids can have holes or be out of order
  std::vector<const Function *> _fixed_side_temperature;

  /// the radiosity of each surface
  std::vector<Real> _radiosity;

  /// the heat flux density qdot
  std::vector<Real> _heat_flux_density;

  /// the average temperature: this could be important for adiabatic walls
  std::vector<Real> _side_temperature;

  /// the type of the side, allows lookup index -> type
  std::vector<enum RAD_BND_TYPE> _side_type;

  /// side id to index map, side ids can have holes or be out of order
  std::map<BoundaryID, unsigned int> _side_id_index;

  /// the area by participating side set
  std::vector<Real> _areas;

  /// the average value of sigma * eps * T^4
  std::vector<Real> _beta;

  /// the irradiation into each surface
  std::vector<Real> _surface_irradiation;

  /// side id to index map for isothermal boundaries, side ids can have holes or be out of order
  std::map<unsigned int, unsigned int> _fixed_side_id_index;

  /// the set of adiabatic boundaries
  std::set<unsigned int> _adiabatic_side_ids;

  /// the view factors which are set by setViewFactors by derived classes
  std::vector<std::vector<Real>> _view_factors;
};
