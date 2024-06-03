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

class StoreVariableByElemIDSideUserObject;
class MeshAlignment2D2D;

/**
 * Computes heat fluxes for HSCoupler2D2DRadiation.
 */
class HSCoupler2D2DRadiationUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  HSCoupler2D2DRadiationUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  /**
   * Gets the heat fluxes for each quadrature point for a given element ID
   *
   * @param[in] elem_id  Element ID
   */
  const std::vector<ADReal> & getHeatFlux(dof_id_type elem_id) const;

protected:
  /// Emissivities of each boundary
  const std::vector<Real> & _emissivities;

  /// View factors between each surface
  const std::vector<std::vector<Real>> & _view_factors;

  /// Whether or not to include an environment surrounding all of the surfaces
  const bool _include_environment;

  /// Environment temperature
  const Real & _T_environment;

  /// User object containing the temperature values on the boundary
  const StoreVariableByElemIDSideUserObject & _temperature_uo;

  /// Mesh alignment object
  const MeshAlignment2D2D & _mesh_alignment;

  /// Number of heat structures
  const unsigned int _n_hs;

  /// Number of surfaces
  const unsigned int _n_surfaces;

  /// Map of the element ID to the heat flux
  std::map<dof_id_type, std::vector<ADReal>> _elem_id_to_heat_flux;
};
