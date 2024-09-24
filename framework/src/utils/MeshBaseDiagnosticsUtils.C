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
checkEdgeOverlap(const std::unique_ptr<Elem> & edge1,
                 const std::unique_ptr<Elem> & edge2,
                 const ConsoleStream & console,
                 const Real intersection_tol)
{
  // get node array from two edges
  const auto node_list1 = edge1->get_nodes();
  const auto node_list2 = edge2->get_nodes();

  // make sure two edges are not the same and don't share any nodes edge before checking for overlap
  auto n1 = *node_list1[0];
  auto n2 = *node_list1[1];
  auto n3 = *node_list2[0];
  auto n4 = *node_list2[1];

  // get x,y,z coordinates for each point
  auto n1x = n1.operator()(0);
  auto n1y = n1.operator()(1);
  auto n1z = n1.operator()(2);
  auto n2x = n2.operator()(0);
  auto n2y = n2.operator()(1);
  auto n2z = n2.operator()(2);
  auto n3x = n3.operator()(0);
  auto n3y = n3.operator()(1);
  auto n3z = n3.operator()(2);
  auto n4x = n4.operator()(0);
  auto n4y = n4.operator()(1);
  auto n4z = n4.operator()(2);

  // Check that none of these points are the same
  if (((std::fabs(n1x - n3x) < intersection_tol) && (std::fabs(n1y - n3y) < intersection_tol) &&
       (std::fabs(n1z - n3z) < intersection_tol)) ||
      ((std::fabs(n2x - n4x) < intersection_tol) && (std::fabs(n2y - n4y) < intersection_tol) &&
       (std::fabs(n2z - n4z) < intersection_tol)) ||
      ((std::fabs(n1x - n4x) < intersection_tol) && (std::fabs(n1y - n4y) < intersection_tol) &&
       (std::fabs(n1z - n4z) < intersection_tol)) ||
      ((std::fabs(n2x - n3x) < intersection_tol) && (std::fabs(n2y - n3y) < intersection_tol) &&
       (std::fabs(n2z - n3z) < intersection_tol)))
    return false;

  /*
    There's a chance that they overlap. Find shortest line that connects two edges and if its length
    is close enough to 0 return true The shortest line between the two edges will be perpendicular
    to both.
  */
  auto d1343 = (n1x - n3x) * (n4x - n3x) + (n1y - n3y) * (n4y - n3y) + (n1z - n3z) * (n4z - n3z);
  auto d4321 = (n4x - n3x) * (n2x - n1x) + (n4y - n3y) * (n2y - n1y) + (n4z - n3z) * (n2z - n1z);
  auto d1321 = (n1x - n3x) * (n2x - n1x) + (n1y - n3y) * (n2y - n1y) + (n1z - n3z) * (n2z - n1z);
  auto d4343 = (n4x - n3x) * (n4x - n3x) + (n4y - n3y) * (n4y - n3y) + (n4z - n3z) * (n4z - n3z);
  auto d2121 = (n2x - n1x) * (n2x - n1x) + (n2y - n1y) * (n2y - n1y) + (n2z - n1z) * (n2z - n1z);

  auto denominator = d2121 * d4343 - d4321 * d4321;
  auto numerator = d1343 * d4321 - d1321 * d4343;

  if (std::fabs(denominator) < intersection_tol)
  {
    // This indicates that the intersecting line is vertical so they don't intersect
    return false;
  }
  auto mua = numerator / denominator;
  auto mub = (d1343 + (mua * d4321)) / d4343;

  // Use these values to solve for the two poits that define the shortest line segment
  auto nax = n1x + mua * (n2x - n1x);
  auto nay = n1y + mua * (n2y - n1y);
  auto naz = n1z + mua * (n2z - n1z);

  auto nbx = n3x + mub * (n4x - n3x);
  auto nby = n3y + mub * (n4y - n3y);
  auto nbz = n3z + mub * (n4z - n3z);

  // This method assume the two lines are infinite. This check to make sure na and nb are part of
  // their respective line segments
  if ((nax < std::min(n1x, n2x)) || (nax > std::max(n1x, n2x)) || (nay < std::min(n1y, n2y)) ||
      (nay > std::max(n1y, n2y)) || (naz < std::min(n1z, n2z)) || (naz > std::max(n1z, n2z)))
  {
    return false;
  }

  if ((nbx < std::min(n3x, n4x)) || (nax > std::max(n3x, n4x)) || (nby < std::min(n3y, n4y)) ||
      (nay > std::max(n3y, n4y)) || (nbz < std::min(n3z, n4z)) || (naz > std::max(n3z, n4z)))
  {
    return false;
  }

  // Calculate distance between these two nodes
  double distance =
      std::sqrt(std::pow(nax - nbx, 2) + std::pow(nay - nby, 2) + std::pow(naz - nbz, 2));
  if (distance < intersection_tol)
  {
    std::string x_coord = std::to_string(nax);
    std::string y_coord = std::to_string(nay);
    std::string z_coord = std::to_string(naz);
    std::string message =
        "Non-matching edges found near (" + x_coord + ", " + y_coord + ", " + z_coord + ")";
    console << message << std::endl;
    return true;
  }
  else
    return false;
}
}
