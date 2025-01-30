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

bool
checkFirstOrderEdgeOverlap(const Elem & edge1,
                           const Elem & edge2,
                           Point & intersection_point,
                           const Real intersection_tol)
{
  // check that the two elements are of type EDGE2
  mooseAssert(edge1.type() == EDGE2, "Elements must be of type EDGE2");
  mooseAssert(edge2.type() == EDGE2, "Elements must be of type EDGE2");
  // Get nodes from the two edges
  const Point & p1 = edge1.point(0);
  const Point & p2 = edge1.point(1);
  const Point & p3 = edge2.point(0);
  const Point & p4 = edge2.point(1);

  // Check that the two edges are not sharing a node
  if (p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4)
    return false;

  /*
    There's a chance that they overlap. Find shortest line that connects two edges and if its length
    is close enough to 0 return true The shortest line between the two edges will be perpendicular
    to both.
  */
  const auto d1343 = (p1 - p3) * (p4 - p3);
  const auto d4321 = (p4 - p3) * (p2 - p1);
  const auto d1321 = (p1 - p3) * (p2 - p1);
  const auto d4343 = (p4 - p3) * (p4 - p3);
  const auto d2121 = (p2 - p1) * (p2 - p1);

  const auto denominator = d2121 * d4343 - d4321 * d4321;
  const auto numerator = d1343 * d4321 - d1321 * d4343;

  if (std::fabs(denominator) < intersection_tol)
    // This indicates that the two lines are parallel so they don't intersect
    return false;

  const auto mua = numerator / denominator;
  const auto mub = (d1343 + (mua * d4321)) / d4343;

  // Use these values to solve for the two points that define the shortest line segment
  const auto pa = p1 + mua * (p2 - p1);
  const auto pb = p3 + mub * (p4 - p3);

  // This method assume the two lines are infinite. This check to make sure na and nb are part of
  // their respective line segments
  if (mua < 0 || mua > 1)
    return false;
  if (mub < 0 || mub > 1)
    return false;

  // Calculate distance between these two nodes
  const auto distance = (pa - pb).norm();
  if (distance < intersection_tol)
  {
    intersection_point = pa;
    return true;
  }
  else
    return false;
}
}
