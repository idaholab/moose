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
#include "ADFunctorInterface.h"

/**
 * Base class for caching quantities computed between flow channels and heat structures.
 */
class FlowChannelHeatStructureCouplerUserObject : public ElementUserObject,
                                                  public ADFunctorInterface
{
public:
  FlowChannelHeatStructureCouplerUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

  /**
   * Get the nearest element ID for given element ID, for either heat structure or flow channel
   *
   * @param elem_id   Element ID from either a flow channel or a heat structure
   * @return Nearest element ID
   */
  const dof_id_type & getNearestElem(dof_id_type elem_id) const
  {
    return _mesh_alignment.getCoupledElemID(elem_id);
  }

protected:
  /**
   * Gets a cached quantity from a map
   */
  const std::vector<ADReal> &
  getCachedQuantity(dof_id_type elem_id,
                    const std::map<dof_id_type, std::vector<ADReal>> & elem_id_to_values,
                    const std::string & description) const;

  /**
   * Gets the cached quantity maps
   */
  virtual std::vector<std::map<dof_id_type, std::vector<ADReal>> *> getCachedQuantityMaps() = 0;
  virtual std::vector<const std::map<dof_id_type, std::vector<ADReal>> *>
  getCachedQuantityMaps() const = 0;

  /// Current flow channel element ID
  dof_id_type _fc_elem_id;
  /// Current heat structure element ID
  dof_id_type _hs_elem_id;
  /// Current flow channel quadrature point index
  unsigned int _fc_qp;
  /// Current heat structure quadrature point index
  unsigned int _hs_qp;

private:
  /// Mesh alignment object
  MeshAlignment & _mesh_alignment;

  /**
   * Computes the cached quantities at a quadrature point
   */
  virtual void computeQpCachedQuantities() = 0;

public:
  static InputParameters validParams();
};
