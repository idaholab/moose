//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowChannelHeatStructureCouplerUserObject.h"

/**
 * Base class for caching heat flux between a flow channel and a heat structure.
 *
 * We provide an API for child classes that they need to implement:
 * 1. computeQpHeatFlux() to compute heat flux at a quadrature point
 * 2. computeQpHeatFluxJacobian() to compute the jacobian of the heat flux
 *    computed by the computeQpHeatFlux() method.
 *
 * There are 2 different clients to the values we cached:
 * 1. BoundaryFluxXYZBC to apply the heat flux on a heat structure boundary
 * 2. OneDXYZEnergyHeatFlux to apply the heat flux on the flow channel side.
 */
class ADHeatFluxFromHeatStructureBaseUserObject : public FlowChannelHeatStructureCouplerUserObject
{
public:
  ADHeatFluxFromHeatStructureBaseUserObject(const InputParameters & parameters);

  const std::vector<ADReal> & getHeatedPerimeter(dof_id_type element_id) const;
  const std::vector<ADReal> & getHeatFlux(dof_id_type element_id) const;

protected:
  virtual std::vector<std::map<dof_id_type, std::vector<ADReal>> *>
  getCachedQuantityMaps() override;
  virtual std::vector<const std::map<dof_id_type, std::vector<ADReal>> *>
  getCachedQuantityMaps() const override;

  virtual ADReal computeQpHeatFlux() = 0;

  /// Flow channel quadrature point index
  unsigned int _qp;
  /// Cached heated perimeter
  std::map<dof_id_type, std::vector<ADReal>> _heated_perimeter;
  /// Cached heat flux
  std::map<dof_id_type, std::vector<ADReal>> _heat_flux;
  /// Coupled heated perimeter variable
  const ADVariableValue & _P_hf;

private:
  virtual void computeQpCachedQuantities() override;

public:
  static InputParameters validParams();
};
