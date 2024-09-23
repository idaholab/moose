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
                 const ConsoleStream & console)
{
  //get node array from two edges
  const auto node_list1 = edge1->get_nodes();
  const auto node_list2 = edge2->get_nodes();

  //make sure two edges are not the same and don't share any nodes edge before checking for overlap
  auto n1 = *node_list1[0];
  auto n2 = *node_list1[1];
  auto n3 = *node_list2[0];
  auto n4 = *node_list2[1];

  auto n1x = 1.0*(n1.operator()(0));
  auto n1y = 1.0*(n1.operator()(1));
  auto n1z = 1.0*(n1.operator()(2));
  auto n2x = 1.0*(n2.operator()(0));
  auto n2y = 1.0*(n2.operator()(1));
  auto n2z = 1.0*(n2.operator()(2));
  auto n3x = 1.0*(n3.operator()(0));
  auto n3y = 1.0*(n3.operator()(1));
  auto n3z = 1.0*(n3.operator()(2));
  auto n4x = 1.0*(n4.operator()(0));
  auto n4y = 1.0*(n4.operator()(1));
  auto n4z = 1.0*(n4.operator()(2));

  //Tolerance should be based on edge length. Here it is arbitrarily set to 1/1000th of the average of the two edge lengths
  auto edge1Length = std::sqrt(std::pow(n1x - n2x, 2) + std::pow(n1y - n2y, 2) + std::pow(n1z - n2z, 2));
  auto edge2Length = std::sqrt(std::pow(n3x - n4x, 2) + std::pow(n3y - n4y, 2) + std::pow(n3z - n4z, 2));
  auto tol = ((edge1Length + edge2Length)/2)/1000;
  //double n13x, n13y, n13z, n21x, n21y, n21z, n43x, n43y, n43z;
  auto n13x = n1x - n3x;
  auto n13y = n1y - n3y;
  auto n13z = n1z - n3z;
  auto n21x = n2x - n1x;
  auto n21y = n2y - n1y;
  auto n21z = n2z - n1z;
  auto n43x = n4x - n3x;
  auto n43y = n4y - n3y;
  auto n43z = n4z - n3z;

  if(((std::fabs(n1x - n3x)<tol) && (std::fabs(n1y - n3y)<tol) && (std::fabs(n1z - n3z)<tol)) ||
    ((std::fabs(n2x-n4x)<tol) && (std::fabs(n2y-n4y)<tol) && (std::fabs(n2z-n4z)<tol)) ||
    ((std::fabs(n1x-n4x)<tol) && (std::fabs(n1y-n4y)<tol) && (std::fabs(n1z-n4z)<tol)) ||
    ((std::fabs(n2x-n3x)<tol) && (std::fabs(n2y-n3y)<tol) && (std::fabs(n2z-n3z)<tol)))
    return false;

  //There's a chance that they overlap. Find shortest line that connects two edges and if its length is close enough to 0 return true
  double d1343, d4321, d1321, d4343, d2121, numerator, denominator, mua, mub;

  d1343 = n13x * n43x + n13y * n43y + n13z * n43z;
  d4321 = n43x * n21x + n43y * n21y + n43z * n21z;
  d1321 = n13x * n21x + n13y * n21y + n13z * n21z;
  d4343 = n43x * n43x + n43y * n43y + n43z * n43z;
  d2121 = n21x * n21x + n21y * n21y + n21z * n21z;

  denominator = d2121 * d4343 - d4321 * d4321;
  numerator = d1343 * d4321 - d1321 * d4343;

  if(std::fabs(denominator) < tol)
  {
    //This indicates that the intersecting line is vertical so they don't intersect
    return false;
  }
  mua = numerator/denominator;
  mub = (d1343 + (mua * d4321)) / d4343;

  //Use these values to solve for the two poits that define the shortest line segment
  double nax, nay, naz, nbx, nby, nbz;
  nax = n1x + mua * n21x;
  nay = n1y + mua * n21y;
  naz = n1z + mua * n21z;

  nbx = n3x + mub * n43x;
  nby = n3y + mub * n43y;
  nbz = n3z + mub * n43z;

  //This method assume the two lines are infinite. This check to make sure na and nb are part of their respective line segments
  if((nax < std::min(n1x, n2x)) || (nax > std::max(n1x, n2x)) ||
     (nay < std::min(n1y, n2y)) || (nay > std::max(n1y, n2y)) ||
     (naz < std::min(n1z, n2z)) || (naz > std::max(n1z, n2z)))
  {
    return false;
  }

  if((nbx < std::min(n3x, n4x)) || (nax > std::max(n3x, n4x)) ||
     (nby < std::min(n3y, n4y)) || (nay > std::max(n3y, n4y)) ||
     (nbz < std::min(n3z, n4z)) || (naz > std::max(n3z, n4z)))
  {
    return false;
  }

  //Calculate distance between these two nodes
  double distance = std::sqrt(std::pow(nax - nbx, 2) + std::pow(nay - nby, 2) + std::pow(naz - nbz, 2));
  if (distance < tol)
  {
    std::string x_coord = std::to_string(nax);
    std::string y_coord = std::to_string(nay);
    std::string z_coord = std::to_string(naz);
    std::string message = "Non-matching edges found near (" + x_coord + ", " + y_coord + ", " + z_coord + ")";
    console << message << std::endl;
    return true;
  }
  else
    return false;
}
}
