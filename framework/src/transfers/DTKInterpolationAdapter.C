// The libMesh Finite Element Library.
// Copyright (C) 2002-2012 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "Moose.h"

#include "DTKInterpolationEvaluator.h"
#include "DTKInterpolationAdapter.h"

#include "libmesh/mesh.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"

#include <DTK_MeshTypes.hpp>
#include <Teuchos_DefaultMpiComm.hpp>

#include <vector>

namespace libMesh
{

DTKInterpolationAdapter::DTKInterpolationAdapter(Teuchos::RCP<const Teuchos::MpiComm<int> > in_comm, EquationSystems & in_es, const Point & offset):
    comm(in_comm),
    es(in_es),
    _offset(offset),
    mesh(in_es.get_mesh()),
    dim(mesh.mesh_dimension())
{
  MPI_Comm old_comm = Moose::swapLibMeshComm(*comm->getRawMpiComm());

  std::set<unsigned int> semi_local_nodes;
  get_semi_local_nodes(semi_local_nodes);

  num_local_nodes = semi_local_nodes.size();

  vertices.resize(num_local_nodes);
  Teuchos::ArrayRCP<double> coordinates(num_local_nodes * dim);

  // Fill in the vertices and coordinates
  {
    unsigned int i = 0;

    for(std::set<unsigned int>::iterator it = semi_local_nodes.begin();
        it != semi_local_nodes.end();
        ++it)
    {
      const Node & node = mesh.node(*it);

      vertices[i] = node.id();

      for(unsigned int j=0; j<dim; j++)
        coordinates[(j*num_local_nodes) + i] = node(j) + offset(j);

      i++;
    }
  }

  // Currently assuming all elements are the same!
  DataTransferKit::DTK_ElementTopology element_topology = get_element_topology(mesh.elem(0));
  unsigned int n_nodes_per_elem = mesh.elem(0)->n_nodes();

  unsigned int n_local_elem = mesh.n_local_elem();

  Teuchos::ArrayRCP<int> elements(n_local_elem);
  Teuchos::ArrayRCP<int> connectivity(n_nodes_per_elem*n_local_elem);

  // Fill in the elements and connectivity
  {
    unsigned int i = 0;

    MeshBase::const_element_iterator end = mesh.local_elements_end();
    for(MeshBase::const_element_iterator it = mesh.local_elements_begin();
        it != end;
        ++it)
    {
      const Elem & elem = *(*it);
      elements[i] = elem.id();

      for(unsigned int j=0; j<n_nodes_per_elem; j++)
        connectivity[(j*n_local_elem)+i] = elem.node(j);

      i++;
    }
  }

  Teuchos::ArrayRCP<int> permutation_list(n_nodes_per_elem);
  for ( int i = 0; i < n_nodes_per_elem; ++i )
    permutation_list[i] = i;

  /*
  if(libMesh::processor_id() == 1)
    sleep(1);

  std::cout<<"n_nodes_per_elem: "<<n_nodes_per_elem<<std::endl;

  std::cout<<"Dim: "<<dim<<std::endl;

  std::cerr<<"Vertices size: "<<vertices.size()<<std::endl;
  {
    std::cerr<<libMesh::processor_id()<<" Vertices: ";

    for(unsigned int i=0; i<vertices.size(); i++)
      std::cerr<<vertices[i]<<" ";

    std::cerr<<std::endl;
  }

  std::cerr<<"Coordinates size: "<<coordinates.size()<<std::endl;
  {
    std::cerr<<libMesh::processor_id()<<" Coordinates: ";

    for(unsigned int i=0; i<coordinates.size(); i++)
      std::cerr<<coordinates[i]<<" ";

    std::cerr<<std::endl;
  }

  std::cerr<<"Connectivity size: "<<connectivity.size()<<std::endl;
  {
    std::cerr<<libMesh::processor_id()<<" Connectivity: ";

    for(unsigned int i=0; i<connectivity.size(); i++)
      std::cerr<<connectivity[i]<<" ";

    std::cerr<<std::endl;
  }

  std::cerr<<"Permutation_List size: "<<permutation_list.size()<<std::endl;
  {
    std::cerr<<libMesh::processor_id()<<" Permutation_List: ";

    for(unsigned int i=0; i<permutation_list.size(); i++)
      std::cerr<<permutation_list[i]<<" ";

    std::cerr<<std::endl;
  }

  */
  Teuchos::RCP<MeshContainerType> mesh_container = Teuchos::rcp(
    new MeshContainerType(dim, vertices, coordinates,
                          element_topology, n_nodes_per_elem,
                          elements, connectivity, permutation_list) );

  // We only have 1 element topology in this grid so we make just one mesh block
  Teuchos::ArrayRCP<Teuchos::RCP<MeshContainerType> > mesh_blocks(1);
  mesh_blocks[0] = mesh_container;

  // Create the MeshManager
  mesh_manager = Teuchos::rcp(new DataTransferKit::MeshManager<MeshContainerType>(mesh_blocks, comm, dim) );

  // Pack the coordinates into a field, this will be the positions we'll ask for other systems fields at
  target_coords = Teuchos::rcp(new DataTransferKit::FieldManager<MeshContainerType>(mesh_container, comm));

  // Swap back
  Moose::swapLibMeshComm(old_comm);
}

DTKInterpolationAdapter::RCP_Evaluator
DTKInterpolationAdapter::get_variable_evaluator(std::string var_name)
{
  if(evaluators.find(var_name) == evaluators.end()) // We haven't created an evaluator for the variable yet
  {
    System * sys = find_sys(var_name);

    // Create the FieldEvaluator
    evaluators[var_name] = Teuchos::rcp(new DTKInterpolationEvaluator(*sys, var_name, _offset));
  }

  return evaluators[var_name];
}

Teuchos::RCP<DataTransferKit::FieldManager<DTKInterpolationAdapter::FieldContainerType> >
DTKInterpolationAdapter::get_values_to_fill(std::string var_name)
{
  if(values_to_fill.find(var_name) == values_to_fill.end())
  {
    Teuchos::ArrayRCP<double> data_space(num_local_nodes);
    Teuchos::RCP<FieldContainerType> field_container = Teuchos::rcp(new FieldContainerType(data_space, 1));
    values_to_fill[var_name] = Teuchos::rcp(new DataTransferKit::FieldManager<FieldContainerType>(field_container, comm));
  }

  return values_to_fill[var_name];
}

void
DTKInterpolationAdapter::update_variable_values(std::string var_name, Teuchos::ArrayView<int> missed_points)
{
  MPI_Comm old_comm = Moose::swapLibMeshComm(*comm->getRawMpiComm());

  System * sys = find_sys(var_name);
  unsigned int var_num = sys->variable_number(var_name);

  Teuchos::RCP<FieldContainerType> values = values_to_fill[var_name]->field();

  // Create a vector containing true or false for each point saying whether it was missed or not
  // We're only going to update values for points that were not missed
  std::vector<bool> missed(values->size(), false);

  for(Teuchos::ArrayView<const int>::const_iterator i=missed_points.begin();
      i != missed_points.end();
      ++i)
    missed[*i] = true;

  unsigned int i=0;
  // Loop over the values (one for each node) and assign the value of this variable at each node
  for(FieldContainerType::iterator it=values->begin(); it != values->end(); ++it)
  {
    // If this point "missed" then skip it
    if(missed[i])
    {
      i++;
      continue;
    }

    unsigned int node_num = vertices[i];
    const Node & node = mesh.node(node_num);

    if(node.processor_id() == libMesh::processor_id())
    {
      // The 0 is for the component... this only works for LAGRANGE!
      dof_id_type dof = node.dof_number(sys->number(), var_num, 0);
      sys->solution->set(dof, *it);
    }

    i++;
  }

  sys->solution->close();

  // Swap back
  Moose::swapLibMeshComm(old_comm);
}


/**
 * Small helper function for finding the system containing the variable.
 *
 * Note that this implies that variable names are unique across all systems!
 */
System *
DTKInterpolationAdapter::find_sys(std::string var_name)
{
  System * sys = NULL;

  // Find the system this variable is from
  for(unsigned int i=0; i<es.n_systems(); i++)
  {
    if(es.get_system(i).has_variable(var_name))
    {
      sys = &es.get_system(i);
      break;
    }
  }

  libmesh_assert(sys);

  return sys;
}

DataTransferKit::DTK_ElementTopology
DTKInterpolationAdapter::get_element_topology(const Elem * elem)
{
  ElemType type = elem->type();

  if(type == EDGE2)
    return DataTransferKit::DTK_LINE_SEGMENT;
  else if(type == TRI3)
    return DataTransferKit::DTK_TRIANGLE;
  else if(type == QUAD4)
    return DataTransferKit::DTK_QUADRILATERAL;
  else if(type == TET4)
    return DataTransferKit::DTK_TETRAHEDRON;
  else if(type == HEX8)
    return DataTransferKit::DTK_HEXAHEDRON;
  else if(type == PYRAMID5)
    return DataTransferKit::DTK_PYRAMID;

  libMesh::err<<"Element type not supported by DTK!"<<std::endl;
  libmesh_error();
}

void
DTKInterpolationAdapter::get_semi_local_nodes(std::set<unsigned int> & semi_local_nodes)
{
  MeshBase::const_element_iterator end = mesh.local_elements_end();
  for(MeshBase::const_element_iterator it = mesh.local_elements_begin();
      it != end;
      ++it)
  {
    const Elem & elem = *(*it);

    for(unsigned int j=0; j<elem.n_nodes(); j++)
      semi_local_nodes.insert(elem.node(j));
  }
}

} // namespace libMesh

#endif // #ifdef LIBMESH_HAVE_DTK
