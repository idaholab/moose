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

#include "MeshExtruder.h"
#include "Moose.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "mesh.h"
#include "boundary_info.h"
#include "elem.h"
#include "cell_prism6.h"
#include "cell_hex8.h"
#include "node.h"

// Static Initialization
const unsigned int MeshExtruder::Quad4_to_Hex8_side_map[4] = {1,2,3,4};
const unsigned int MeshExtruder::Tri3_to_Prism6_side_map[3] = {1,2,3};


template<>
InputParameters validParams<MeshExtruder>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addRequiredParam<std::string>("file", "The 2D Mesh file to extrude");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers in the extruded mesh");
  params.addRequiredParam<Real>("height", "The height of the Mesh");
  params.addRequiredParam<unsigned int>("extrusion_axis", "The axis that the mesh will be extruded upon");
  params.addParam<std::vector<unsigned int> >("bottom_sidesets", "A list of boundaries that will be applied to the bottom of the extruded mesh");
  params.addParam<std::vector<unsigned int> >("top_sidesets", "A list of boundaries that will be applied to the top of the extruded mesh");
  return params;
}

MeshExtruder::MeshExtruder(const std::string & name, InputParameters parameters):
    MooseMesh(name, parameters),
    _num_layers(getParam<unsigned int>("num_layers")),
    _height(getParam<Real>("height")),
    _extrusion_axis(getParam<unsigned int>("extrusion_axis"))
{
  // Read in the 2D Mesh
  _src_mesh.read(getParam<std::string>("file"));

  _src_mesh.prepare_for_use();

  extrude(_mesh);
}

void
MeshExtruder::extrude(libMesh::MeshBase &dest_mesh)
{
  const Real layer_ht = _height/_num_layers;

  MeshBase::const_node_iterator nd_end = _src_mesh.active_nodes_end();
  MeshBase::const_element_iterator el_end = _src_mesh.active_elements_end();

  unsigned int n_active_nodes = _src_mesh.n_nodes();
  mooseAssert(_src_mesh.mesh_dimension() == 2, "Can only extrude 2D meshes (right now)");
  dest_mesh.set_mesh_dimension(_src_mesh.mesh_dimension()+1);
  for (unsigned int layer_n = 0; layer_n <= _num_layers; ++layer_n)
  {
    // Add the extruded nodes to the new mesh
    MeshBase::const_node_iterator nd     = _src_mesh.active_nodes_begin();

    for ( ; nd != nd_end; ++nd)
    {
      Node *new_node = new Node(**nd);

      new_node->set_id((*nd)->id() + layer_n * n_active_nodes);

      // Update the extruded coordinate
      (*new_node)(_extrusion_axis) = (*new_node)(_extrusion_axis) + layer_n * layer_ht;

      dest_mesh.add_node(new_node);
    }
  }

  for (unsigned int layer_n = 0; layer_n <= _num_layers; ++layer_n)
  {
    // Add the extruded elements to the new mesh
    MeshBase::const_element_iterator el = _src_mesh.active_elements_begin();

    // We have one less layer of elements than layers of nodes
    if (layer_n < _num_layers)
    {
      for ( ; el != el_end; ++el)
      {
        Elem *elem;
        unsigned int top_side_num;
        switch ((*el)->type())
        {
        case QUAD4:
          elem = dest_mesh.add_elem(new Hex8);
          elem->subdomain_id() = (*el)->subdomain_id();
          top_side_num = 5;

          // The first four nodes are on the current level
          for (unsigned int node_num = 0; node_num < 4; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num)->id() + layer_n * n_active_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }
          // The next four nodes are on the next level
          for (unsigned int node_num = 4; node_num < 8; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num-4)->id() + (layer_n+1) * n_active_nodes;
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
          top_side_num = 4;

          // The first thre nodes are on the current level
          for (unsigned int node_num = 0; node_num < 3; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num)->id() + layer_n * n_active_nodes;
            elem->set_node(node_num) = dest_mesh.node_ptr(dest_node_id);
          }
          // The next three nodes are on the next level
          for (unsigned int node_num = 3; node_num < 6; node_num++)
          {
            unsigned int dest_node_id = (*el)->get_node(node_num-3)->id() + (layer_n+1) * n_active_nodes;
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
          break;
        }

        // bottom sidesets
        if (layer_n == 0 && isParamValid("bottom_sidesets"))
        {
          std::vector<unsigned int> boundaries = getParam<std::vector<unsigned int> >("bottom_sidesets");
          for (std::vector<unsigned int>::const_iterator iter=boundaries.begin(); iter!=boundaries.end(); ++iter)
            dest_mesh.boundary_info->add_side(elem, 0, *iter);
        }
        // top sidesets
        else if (layer_n == _num_layers-1 && isParamValid("top_sidesets"))
        {
          std::vector<unsigned int> boundaries = getParam<std::vector<unsigned int> >("top_sidesets");
          for (std::vector<unsigned int>::const_iterator iter=boundaries.begin(); iter!=boundaries.end(); ++iter)
            dest_mesh.boundary_info->add_side(elem, top_side_num, *iter);
        }
      }
    }
  }
}
