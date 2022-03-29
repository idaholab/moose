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
#include "FlowChannel3DAlignment.h"

/**
 * Caching heat flux data (fluid temperature and heat transfer coefficient)
 * between a flow channel and a 3D heat structure.
 *
 */
class ADHeatTransferFromHeatStructure3D1PhaseUserObject : public ElementUserObject
{
public:
  ADHeatTransferFromHeatStructure3D1PhaseUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  const std::vector<ADReal> & getHeatedPerimeter(dof_id_type element_id) const;
  const std::vector<ADReal> & getHeatTransferCoeff(dof_id_type element_id) const;
  const std::vector<ADReal> & getTfluid(dof_id_type element_id) const;

  /**
   * Get the nearest element ID for given element ID
   *
   * Used when a heat structure element needs to know what its nearest element is and vice versa.
   * @param elem_id Element ID either from a flow channel or a heat structure
   * @return Nearest element corresponding to a `elem_id`
   */
  const dof_id_type & getNearestElem(dof_id_type elem_id) const
  {
    return _fch_alignment.getNearestElemID(elem_id);
  }

protected:
  /**
   * Parallel gather of all local contributions into one global map
   */
  void allGatherMap(std::map<dof_id_type, std::vector<ADReal>> & data);

  /// Flow channel alignment object
  const FlowChannel3DAlignment & _fch_alignment;
  /// Coupled heated perimeter variable
  const ADVariableValue & _P_hf;
  /// Heat transfer coefficient
  const ADMaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  const std::vector<dof_id_type> & _hs_elem_ids;

  /// How qpoint indices are mapped from slave side to master side per element
  std::map<dof_id_type, std::vector<unsigned int>> _elem_qp_map;
  /// Map of the element ID to the heated perimeter at the quadrature points
  std::map<dof_id_type, std::vector<ADReal>> _heated_perimeter;
  /// Map of the element ID to the fluid temperature at the quadrature points
  std::map<dof_id_type, std::vector<ADReal>> _T_fluid;
  /// Map of the element ID to the wall heat transfer coefficient at the quadrature points
  std::map<dof_id_type, std::vector<ADReal>> _htc;

public:
  static InputParameters validParams();
};
