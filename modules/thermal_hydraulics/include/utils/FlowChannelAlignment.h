//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshAlignment.h"
#include "THMMesh.h"

class FlowChannelAlignment : public MeshAlignment
{
public:
  /**
   * @param mesh[in] The mesh
   */
  FlowChannelAlignment(const THMMesh & mesh);

  /**
   * Build the neighborhood information between master and slave side
   *
   * @param master_boundary_info[in] List of tuples (elem_id, side_id) of the master side (i.e. heat
   * structure side)
   * @param slave_elem_ids[in] List of slave element IDs (i.e. flow channel elements)
   */
  void build(const std::vector<std::tuple<dof_id_type, unsigned short int>> & master_boundary_info,
             const std::vector<dof_id_type> & slave_elem_ids);

  /**
   * Check the alignment of the flow channel elements with the heat structure elements
   *
   * @param slave_elem_ids[in] The vector of flow channel element IDs
   */
  bool check(const std::vector<dof_id_type> & slave_elem_ids) const;

  /**
   * Get the nearest element ID given another element ID
   *
   * @param[in] elem_id Element ID for which we want to obtain the nearest element ID
   */
  const dof_id_type & getNearestElemID(const dof_id_type & elem_id) const;
};
