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
#include "MeshAlignment.h"

/**
 * Base class for caching heat flux between a flow channel and a heat structure.
 *
 * We procide an API for child classes that they need to implement:
 * 1. computeQpHeatFlux() to compute heat flux at a quadrature point
 * 2. computeQpHeatFluxJacobian() to compute the jacobian of the heat flux
 *    computed by the computeQpHeatFlux() method.
 *
 * There are 2 different clients to the values we cached:
 * 1. BoundaryFluxXYZBC to apply the heat flux on a heat structure boundary
 * 2. OneDXYZEnergyHeatFlux to apply the heat flux on the flow channel side.
 */
class HeatFluxFromHeatStructureBaseUserObject : public ElementUserObject
{
public:
  HeatFluxFromHeatStructureBaseUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  const std::vector<Real> & getHeatedPerimeter(dof_id_type element_id) const;
  const std::vector<Real> & getHeatFlux(dof_id_type element_id) const;
  const std::vector<DenseVector<Real>> & getHeatFluxJacobian(dof_id_type element_id) const;

  /**
   * Get the nearest element ID for given element ID
   *
   * Used when a heat structure element needs to know what its nearest element is and vice versa.
   * @param elem_id Element ID either from a flow channel or a heat structure
   * @return Nearest element corresponding to a `elem_id`
   */
  const dof_id_type & getNearestElem(dof_id_type elem_id) const
  {
    return _mesh_alignment.getCoupledElemID(elem_id);
  }

protected:
  virtual Real computeQpHeatFlux() = 0;
  virtual DenseVector<Real> computeQpHeatFluxJacobian() = 0;

  /// Mesh alignment object
  MeshAlignment & _mesh_alignment;
  /// Quadrature point index
  unsigned int _qp;
  /// Cached heated perimeter
  std::map<dof_id_type, std::vector<Real>> _heated_perimeter;
  /// Cached heat flux
  std::map<dof_id_type, std::vector<Real>> _heat_flux;
  /// Cached heat flux jacobians
  std::map<dof_id_type, std::vector<DenseVector<Real>>> _heat_flux_jacobian;
  /// Coupled heated perimeter variable
  const VariableValue & _P_hf;

public:
  static InputParameters validParams();
};
