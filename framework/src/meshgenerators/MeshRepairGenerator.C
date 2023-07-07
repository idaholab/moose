//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshRepairGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", MeshRepairGenerator);

InputParameters
MeshRepairGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "T         ");
  params.addClassDescription("");

  params.addParam<bool>("fix_node_overlap",
                        true,
                        "fixing the mesh by deleting overlapping nodes for stitching capabilities");

  return params;
}

MeshRepairGenerator::MeshRepairGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _fix_overlapping_nodes(getParam<bool>("fix_node_overlap"))
{
}

std::unique_ptr<MeshBase>
MeshRepairGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  mesh->prepare_for_use();
  if (_fix_overlapping_nodes)
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes
    for (auto & node : mesh->local_node_ptr_range())
    {
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      for (auto & elem : elements)
      {
        bool found = false;
        for (auto & elem_node : elem->node_ref_range())
        {
          if ((*node).id() == elem_node.id())
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          for (auto & elem_node : elem->node_ref_range())
          {
            Real tol = 1e-8;
            // Compares the coordinates (add absoluteFuzzyEqual)
            const auto x_node = (*node)(0);
            const auto x_elem_node = elem_node(0);
            const auto y_node = (*node)(1);
            const auto y_elem_node = elem_node(1);
            const auto z_node = (*node)(2);
            const auto z_elem_node = elem_node(2);
            // static_cast<const Point *>(node) == static_cast<const Point>(elem_node)
            if (MooseUtils::absoluteFuzzyEqual(x_node, x_elem_node, tol) &&
                MooseUtils::absoluteFuzzyEqual(y_node, y_elem_node, tol) &&
                MooseUtils::absoluteFuzzyEqual(z_node, z_elem_node, tol))
            {
              // Coordinates are the same but it's not the same node
              // Replace the node in the element
              const_cast<Elem *>(elem)->set_node(elem->get_node_index(&elem_node)) = node;
              _num_fixed_nodes++;
              _console << "Stitch nodes at : " << *node << std::endl;
            }
          }
        }
      }
    }
    _console << "Number of nodes overlapping which got merged: " << _num_fixed_nodes << std::endl;
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}
