//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshComponent.h"
#include "THMMesh.h"
#include "MooseUtils.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshComponent);

InputParameters
FileMeshComponent::validParams()
{
  InputParameters params = GeometricalComponent::validParams();

  params.addRequiredParam<FileName>("file", "The ExodusII mesh file name");
  params.addRequiredParam<Point>("position", "Translation vector for the file mesh [m]");

  params.addClassDescription("Loads a mesh from an ExodusII file without adding physics.");

  return params;
}

FileMeshComponent::FileMeshComponent(const InputParameters & parameters)
  : GeometricalComponent(parameters),
    _file_name(getParam<FileName>("file")),
    _file_is_readable(MooseUtils::pathExists(_file_name) &&
                      MooseUtils::checkFileReadable(_file_name, false, false)),
    _position(getParam<Point>("position"))
{
  // The following is a mooseError instead of logError because the 'add_variable'
  // and 'add_aux_variable' tasks must execute before the integrity check, so if
  // the file is unreadable, the user would just get an error message about variables
  // being added to non-existent blocks.
  if (!_file_is_readable)
    mooseError("The file '",
               _file_name,
               "' could not be opened. Check that the file exists and that "
               "you have permission to open it.");
}

void
FileMeshComponent::setupMesh()
{
  if (_file_is_readable)
  {
    buildMesh();

    // apply translation vector to all nodes
    for (auto && node_id : _node_ids)
    {
      Node & node = mesh().nodeRef(node_id);
      RealVectorValue p(node(0), node(1), node(2));
      node = p + _position;
    }
  }
}

std::vector<std::string>
FileMeshComponent::buildMesh()
{
  std::vector<std::string> subdomain_names;

  auto & thm_mesh = mesh();

  ExodusII_IO_Helper exio_helper(*this, false, true, false);
  exio_helper.open(_file_name.c_str(), true);
  exio_helper.read_and_store_header_info();

  // maps from Exodus IDs to THMMesh IDs
  std::map<int, unsigned int> node_id_map;
  std::map<int, unsigned int> elem_id_map;
  std::map<int, unsigned int> boundary_id_map;

  // Loop over nodes:
  // - Add the nodes into THMMesh
  // - Populate node_id_map
  exio_helper.read_nodes();
  exio_helper.read_node_num_map();
  for (int i = 0; i < exio_helper.num_nodes; i++)
  {
    int exodus_id = exio_helper.node_num_map[i];

    Point p(exio_helper.x[i], exio_helper.y[i], exio_helper.z[i]);
    const Node * node = addNode(p);
    node_id_map[exodus_id] = node->id();
  }

  // Loop over blocks:
  // - Populate subdomain_names
  // - Set the blocks in THMMesh
  // - Add the elements into THMMesh
  exio_helper.read_block_info();
  exio_helper.read_elem_num_map();
  int jmax_last_block = 0;
  for (int i = 0; i < exio_helper.num_elem_blk; i++)
  {
    exio_helper.read_elem_in_block(i);

    // Get subdomain name from file (or ID if no name) and populate subdomain_names
    std::string subdomain_name = exio_helper.get_block_name(i);
    if (subdomain_name.empty())
      subdomain_name = Moose::stringify(exio_helper.get_block_id(i));
    subdomain_names.push_back(subdomain_name);

    // Generate the subdomain name and ID for THMMesh, and set them
    const std::string component_subdomain_name = genName(_name, subdomain_name);
    SubdomainID sid = thm_mesh.getNextSubdomainId();
    setSubdomainInfo(sid, component_subdomain_name, Moose::COORD_XYZ);

    const std::string type_str(exio_helper.get_elem_type());
    const auto & conv = exio_helper.get_conversion(type_str);

    // Loop over elements in block
    int jmax = jmax_last_block + exio_helper.num_elem_this_blk;
    for (int j = jmax_last_block; j < jmax; j++)
    {
      // Loop over nodes on element:
      // - Get the node IDs on the element
      std::vector<dof_id_type> node_ids(exio_helper.num_nodes_per_elem);
      for (int k = 0; k < exio_helper.num_nodes_per_elem; k++)
      {
        int gi = (j - jmax_last_block) * exio_helper.num_nodes_per_elem + conv.get_node_map(k);
        int ex_node_id = exio_helper.node_num_map[exio_helper.connect[gi] - 1];
        node_ids[k] = node_id_map[ex_node_id];
      }

      // Add the element and set its subdomain
      Elem * elem = addElement(conv.libmesh_elem_type(), node_ids);
      elem->subdomain_id() = sid;

      // Populate elem_id_map
      int exodus_id = exio_helper.elem_num_map[j];
      elem_id_map[exodus_id] = elem->id();
    }
    jmax_last_block += exio_helper.num_elem_this_blk;
  }

  // Loop over boundaries:
  // - Get the Exodus boundary names and store them
  // - Generate the boundary IDs
  exio_helper.read_sideset_info();
  int offset = 0;
  std::unordered_map<BoundaryID, BoundaryName> new_ids_to_names;
  for (int i = 0; i < exio_helper.num_side_sets; i++)
  {
    // Compute new offset
    offset += (i > 0 ? exio_helper.num_sides_per_set[i - 1] : 0);
    exio_helper.read_sideset(i, offset);

    // Get boundary name from file (or ID if no name)
    int ex_sideset_id = exio_helper.get_side_set_id(i);
    std::string sideset_name = exio_helper.get_side_set_name(i);
    if (sideset_name.empty())
      sideset_name = Moose::stringify(ex_sideset_id);

    // Generate the boundary ID for THMMesh and populate boundary_id_map
    unsigned int bc_id = thm_mesh.getNextBoundaryId();
    boundary_id_map[ex_sideset_id] = bc_id;
    new_ids_to_names.emplace(bc_id, sideset_name);
  }

  auto & boundary_info = thm_mesh.getMesh().get_boundary_info();

  // Loop over the elements on boundaries
  // Add the boundary elem/side pairs to the boundary
  for (auto e : index_range(exio_helper.elem_list))
  {
    // Get the element
    int ex_elem_id = exio_helper.elem_num_map[exio_helper.elem_list[e] - 1];
    dof_id_type elem_id = elem_id_map[ex_elem_id];
    Elem * elem = thm_mesh.elemPtr(elem_id);

    // Get the side index
    const auto & conv = exio_helper.get_conversion(elem->type());
    // Map the zero-based Exodus side numbering to the libmesh side numbering
    unsigned int raw_side_index = exio_helper.side_list[e] - 1;
    std::size_t side_index_offset = conv.get_shellface_index_offset();
    unsigned int side_index = static_cast<unsigned int>(raw_side_index - side_index_offset);
    int mapped_side = conv.get_side_map(side_index);

    // Get the boundary ID and add the elem/side pair to the boundary
    unsigned int bc_id = boundary_id_map[exio_helper.id_list[e]];
    boundary_info.add_side(elem, mapped_side, bc_id);
  }

  // Generate and set the boundary name
  for (const auto & id_and_name : new_ids_to_names)
  {
    const std::string sideset_name = genName(_name, id_and_name.second);
    boundary_info.sideset_name(id_and_name.first) = sideset_name;
    boundary_info.nodeset_name(id_and_name.first) = sideset_name;
  }

  // This appears to be necessary to get the nodesets named correctly, despite
  // the fact that it gets called later.
  boundary_info.build_node_list_from_side_list();

  return subdomain_names;
}
