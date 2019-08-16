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
class GrayLambertSurfaceRadiation;
class Function;

template <>
InputParameters validParams<GrayLambertSurfaceRadiation>();

/**
 * GrayLambertSurfaceRadiation computes the heat flux on a set of surfaces
 * in radiative heat transfer with each other.
 */
class GrayLambertSurfaceRadiation : public SideUserObject
{
public:
  GrayLambertSurfaceRadiation(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;

  /// Define enum for boundary type
  enum RAD_BND_TYPE
  {
    VARIABLE_TEMPERATURE = 0,
    FIXED_TEMPERATURE = 4,
    ADIABATIC = 8
  };

  ///@{ public interface of this UserObject
  Real getSurfaceHeatFluxDensity(BoundaryID id) const;
  Real getSurfaceTemperature(BoundaryID id) const;
  Real getSurfaceRadiosity(BoundaryID id) const;
  ///@}

protected:
  virtual void threadJoin(const UserObject & y) override;

  /// Stefan-Boltzmann constant
  const Real _sigma_stefan_boltzmann;

  /// number of active boundary ids
  unsigned int _n_sides;

  /// the coupled temperature variable
  const VariableValue & _temperature;

  /// constant emissivity for each boundary
  const std::vector<Real> _emissivity;

  /// the view factors, stored as square array here, but input allows specifying upper triangular part only
  std::vector<std::vector<Real>> _view_factors;

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
  std::map<unsigned int, unsigned int> _side_id_index;

  /// the area by participating side set
  std::vector<Real> _areas;

  /// the average value of sigma * eps * T^4
  std::vector<Real> _beta;

  /// side id to index map for isothermal boundaries, side ids can have holes or be out of order
  std::map<unsigned int, unsigned int> _fixed_side_id_index;

  /// the set of adiabatic boundaries
  std::set<unsigned int> _adiabatic_side_ids;
};
