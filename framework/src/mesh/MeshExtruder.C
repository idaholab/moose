/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Moose.h"
#include "MeshExtruder.h"

#include "boundary_info.h"
#include "elem.h"
#include "cell_prism6.h"
#include "cell_hex8.h"
#include "node.h"

// Static Initialization
const unsigned int MeshExtruder::Quad4_to_Hex8_side_map[4] = {1,2,3,4};
const unsigned int MeshExtruder::Tri3_to_Prism6_side_map[3] = {1,2,3};

MeshExtruder::MeshExtruder(const libMesh::MeshBase &source_mesh):
    _src_mesh(source_mesh)
{
}

void
MeshExtruder::extrude(libMesh::MeshBase &dest_mesh, unsigned int num_layers,
                      unsigned int extrusion_axis, Real height)
{
  const Real layer_ht = height/num_layers;

  MeshBase::const_node_iterator nd_end = _src_mesh.local_nodes_end();
  MeshBase::const_element_iterator el_end = _src_mesh.local_elements_end();

  unsigned int n_local_nodes = _src_mesh.n_local_nodes();
  mooseAssert(_src_mesh.mesh_dimension() == 2, "Can only extrude 2D meshes (right now)");
  dest_mesh.set_mesh_dimension(_src_mesh.mesh_dimension()+1);
  for (unsigned int layer_n = 0; layer_n <= num_layers; ++layer_n)
  {
    // Add the extruded nodes to the new mesh
    MeshBase::const_node_iterator nd     = _src_mesh.local_nodes_begin();

    for ( ; nd != nd_end; ++nd)
    {
      Node *new_node = new Node(**nd);

      // DEBUG
      unsigned int stuff = new_node->id();
      unsigned int stuff2 = new_node->id() + layer_n * n_local_nodes;
      new_node->set_id(new_node->id() + layer_n * n_local_nodes);

      // Update the extruded coordinate
      (*new_node)(extrusion_axis) = (*new_node)(extrusion_axis) + layer_n * layer_ht;

      //std::cout << new_node->id() << ": ";
      //new_node->print();

      dest_mesh.add_node(new_node);
    }
  }

  for (unsigned int layer_n = 0; layer_n <= num_layers; ++layer_n)
  {
    // Add the extruded elements to the new mesh
    MeshBase::const_element_iterator el = _src_mesh.local_elements_begin();

    // We have one less layer of elements than layers of nodes
    if (layer_n < num_layers)
    {
      for ( ; el != el_end; ++el)
      {
        Elem *elem;
        switch ((*el)->type())
        {
        case QUAD4:
          elem = dest_mesh.add_elem(new Hex8);
          elem->subdomain_id() = (*el)->subdomain_id();

          // The first four nodes are on the current level
          for (unsigned int node_num = 0; node_num < 4; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num)->id() + layer_n * n_local_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }
          // The next four nodes are on the next level
          for (unsigned int node_num = 4; node_num < 8; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num-4)->id() + (layer_n+1) * n_local_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }

          // Propogate sides
          for (short int side = 0; side < 4; ++side)
          {
            std::vector<short int> boundary_ids = _src_mesh.boundary_info->boundary_ids(*el, side);

            if (!boundary_ids.empty())
              dest_mesh.boundary_info->add_side(elem, Quad4_to_Hex8_side_map[side], boundary_ids);
          }


          break;
        case TRI3:
          elem = dest_mesh.add_elem(new Prism6);
          elem->subdomain_id() = (*el)->subdomain_id();

          // The first thre nodes are on the current level
          for (unsigned int node_num = 0; node_num < 3; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num)->id() + layer_n * n_local_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }
          // The next three nodes are on the next level
          for (unsigned int node_num = 3; node_num < 6; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num-3)->id() + (layer_n+1) * n_local_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }

          // Propogate sides
          for (short int side = 0; side < 4; ++side)
          {
            std::vector<short int> boundary_ids = _src_mesh.boundary_info->boundary_ids(*el, side);

            if (!boundary_ids.empty())
              dest_mesh.boundary_info->add_side(elem, Tri3_to_Prism6_side_map[side], boundary_ids);
          }


          break;
        default:
          std::cout << ((*el)->type());

          mooseError("Element type not setup");
        }
      }
    }
  }
}
