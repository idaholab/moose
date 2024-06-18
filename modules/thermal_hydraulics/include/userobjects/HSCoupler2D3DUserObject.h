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
class MeshAlignment2D3D;

/**
 * Computes heat fluxes for HSCoupler2D3D.
 */
class HSCoupler2D3DUserObject : public SideUserObject
{
public:
  static InputParameters validParams();

  HSCoupler2D3DUserObject(const InputParameters & parameters);

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
  /**
   * Evaluates a function of temperature
   *
   * @param[in] fn  Function of temperature
   * @param[in] T  Temperature at which to evaluate function
   */
  ADReal evaluateTemperatureFunction(const Function & fn, const ADReal & T) const;

  /// Coupled heated perimeter variable
  const ADVariableValue & _T_3d;

  /// Radius of the 2D heat structure boundary
  const Real & _r_2d;

  /// Emissivity of the 2D heat structure boundary as a function of temperature
  const Function & _emissivity_2d_fn;
  /// Emissivity of the 3D heat structure boundary as a function of temperature
  const Function & _emissivity_3d_fn;
  /// Include radiation?
  const bool _include_radiation;
  /// Gap thickness as a function of temperature
  const Function & _gap_thickness_fn;
  /// Gap thermal conductivity as a function of temperature
  const Function & _k_gap_fn;
  /// Gap heat transfer coefficient as a function of temperature
  const Function & _htc_gap_fn;

  /// User object containing the temperature values on the 2D boundary
  const StoreVariableByElemIDSideUserObject & _temperature_2d_uo;

  /// Mesh alignment object
  MeshAlignment2D3D & _mesh_alignment;

  /// Map of the element ID to the heat flux
  std::map<dof_id_type, std::vector<ADReal>> _elem_id_to_heat_flux;
};
