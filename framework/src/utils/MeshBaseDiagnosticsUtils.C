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
checkFirstOrderEdgeOverlap(const std::unique_ptr<Elem> & edge1,
                           const std::unique_ptr<Elem> & edge2,
                           std::vector<double> & intersection_coords,
                           const Real intersection_tol)
{
  // check that the two elements are of type EDGE2
  mooseAssert(edge1->type() == 0,
              "Elements must be of type EDGE2");
  mooseAssert(edge2->type() == 0,
              "Elements must be of type EDGE2");
  // Get nodes from the two edges
  const Node * const n1 = edge1->get_nodes()[0];
  const Node * const n2 = edge1->get_nodes()[1];
  const Node * const n3 = edge2->get_nodes()[0];
  const Node * const n4 = edge2->get_nodes()[1];

  // get x,y,z coordinates for each node
  const auto n1x = n1->operator()(0);
  const auto n1y = n1->operator()(1);
  const auto n1z = n1->operator()(2);
  const auto n2x = n2->operator()(0);
  const auto n2y = n2->operator()(1);
  const auto n2z = n2->operator()(2);
  const auto n3x = n3->operator()(0);
  const auto n3y = n3->operator()(1);
  const auto n3z = n3->operator()(2);
  const auto n4x = n4->operator()(0);
  const auto n4y = n4->operator()(1);
  const auto n4z = n4->operator()(2);

  // Check that the two edges are not sharing a node
  if (n1->id() == n3->id() || n1->id() == n4->id() || n2->id() == n3->id() || n2->id() == n4->id())
    return false;

  /*
    There's a chance that they overlap. Find shortest line that connects two edges and if its length
    is close enough to 0 return true The shortest line between the two edges will be perpendicular
    to both.
  */
  const auto d1343 =
      (n1x - n3x) * (n4x - n3x) + (n1y - n3y) * (n4y - n3y) + (n1z - n3z) * (n4z - n3z);
  const auto d4321 =
      (n4x - n3x) * (n2x - n1x) + (n4y - n3y) * (n2y - n1y) + (n4z - n3z) * (n2z - n1z);
  const auto d1321 =
      (n1x - n3x) * (n2x - n1x) + (n1y - n3y) * (n2y - n1y) + (n1z - n3z) * (n2z - n1z);
  const auto d4343 =
      (n4x - n3x) * (n4x - n3x) + (n4y - n3y) * (n4y - n3y) + (n4z - n3z) * (n4z - n3z);
  const auto d2121 =
      (n2x - n1x) * (n2x - n1x) + (n2y - n1y) * (n2y - n1y) + (n2z - n1z) * (n2z - n1z);

  const auto denominator = d2121 * d4343 - d4321 * d4321;
  const auto numerator = d1343 * d4321 - d1321 * d4343;

  if (std::fabs(denominator) < intersection_tol)
    // This indicates that the two lines are parallel so they don't intersect
    return false;

  const auto mua = numerator / denominator;
  const auto mub = (d1343 + (mua * d4321)) / d4343;

  // Use these values to solve for the two points that define the shortest line segment
  const auto nax = n1x + mua * (n2x - n1x);
  const auto nay = n1y + mua * (n2y - n1y);
  const auto naz = n1z + mua * (n2z - n1z);

  const auto nbx = n3x + mub * (n4x - n3x);
  const auto nby = n3y + mub * (n4y - n3y);
  const auto nbz = n3z + mub * (n4z - n3z);

  // This method assume the two lines are infinite. This check to make sure na and nb are part of
  // their respective line segments
  if (mua < 0 || mua > 1)
    return false;
  if (mub < 0 || mub > 1)
    return false;

  // Calculate distance between these two nodes
  const auto distance = std::sqrt(Utility::pow<2>(nax - nbx) + Utility::pow<2>(nay - nby) +
                                  Utility::pow<2>(naz - nbz));
  if (distance < intersection_tol)
  {
    intersection_coords[0] = nax;
    intersection_coords[1] = nay;
    intersection_coords[2] = naz;
    return true;
  }
  else
    return false;
}
}
