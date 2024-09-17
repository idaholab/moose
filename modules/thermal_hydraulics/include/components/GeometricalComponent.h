//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"
#include "libmesh/enum_elem_type.h"

/**
 * Intermediate class for components that have mesh
 */
class GeometricalComponent : public Component
{
public:
  GeometricalComponent(const InputParameters & parameters);

protected:
  Elem * addElement(libMesh::ElemType elem_type, const std::vector<dof_id_type> & node_ids);
  Elem * addElementEdge2(dof_id_type node0, dof_id_type node1);
  Elem * addElementEdge3(dof_id_type node0, dof_id_type node1, dof_id_type node2);
  Elem *
  addElementQuad4(dof_id_type node0, dof_id_type node1, dof_id_type node2, dof_id_type node3);
  Elem * addElementQuad9(dof_id_type node0,
                         dof_id_type node1,
                         dof_id_type node2,
                         dof_id_type node3,
                         dof_id_type node4,
                         dof_id_type node5,
                         dof_id_type node6,
                         dof_id_type node7,
                         dof_id_type node8);

  /**
   * Makes a constant function parameter controllable and returns its name
   *
   * @param[in] fn_param_name   FunctionName parameter
   */
  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

public:
  static InputParameters validParams();
};
