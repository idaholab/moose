//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MeshBaseDiagnosticsUtils.h"
#include "ConsoleStream.h"
#include "MooseError.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point_locator_base.h"

namespace MeshBaseDiagnosticsUtils
{
void
checkNonConformalMesh(const std::unique_ptr<MeshBase> & mesh,
                      const ConsoleStream & console,
                      const unsigned int num_outputs,
                      const Real conformality_tol,
                      unsigned int & num_nonconformal_nodes)
{
  num_nonconformal_nodes = 0;
  auto pl = mesh->sub_point_locator();
  pl->set_close_to_point_tol(conformality_tol);

  if (!mesh->is_serial())
    mooseError("Only serialized/replicated meshes are supported");

  // loop on nodes, assumes a replicated mesh
  for (auto & node : mesh->node_ptr_range())
  {
    // find all the elements around this node
    std::set<const Elem *> elements;
    (*pl)(*node, elements);

    // loop through the set of elements near this node
    for (auto & elem : elements)
    {
      // If the node is not part of this element's nodes, it is a
      // case of non-conformality
      bool found_conformal = false;

      for (auto & elem_node : elem->node_ref_range())
      {
        if (*node == elem_node)
        {
          found_conformal = true;
          break;
        }
      }
      if (!found_conformal)
      {
        num_nonconformal_nodes++;
        if (num_nonconformal_nodes < num_outputs)
          console << "Non-conformality detected at  : " << *node << std::endl;
        else if (num_nonconformal_nodes == num_outputs)
          console << "Maximum output reached, log is silenced" << std::endl;
      }
    }
  }
  pl->unset_close_to_point_tol();
}
}
