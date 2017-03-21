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

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

// MOOSE includes
#include "Moose.h"
#include "DTKInterpolationEvaluator.h"
#include "DTKInterpolationAdapter.h"
#include "Transfer.h"

// libMesh includes
#include "libmesh/mesh.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem.h"
#include "libmesh/equation_systems.h"

// DTK includes
#include "libmesh/ignore_warnings.h"
#include <DTK_MeshTypes.hpp>
#include "libmesh/restore_warnings.h"

DTKInterpolationAdapter::DTKInterpolationAdapter(Teuchos::RCP<const Teuchos::MpiComm<int>> in_comm,
                                                 EquationSystems & in_es,
                                                 const Point & offset,
                                                 unsigned int from_dim)
  : comm(in_comm), es(in_es), _offset(offset), mesh(in_es.get_mesh()), dim(mesh.mesh_dimension())
{
  MPI_Comm old_comm = Moose::swapLibMeshComm(*comm->getRawMpiComm());

  std::set<GlobalOrdinal> semi_local_nodes;
  get_semi_local_nodes(semi_local_nodes);

  num_local_nodes = semi_local_nodes.size();

  vertices.resize(num_local_nodes);
  Teuchos::ArrayRCP<double> coordinates(num_local_nodes * dim);

  Teuchos::ArrayRCP<double> target_coordinates(num_local_nodes * from_dim);

  // Fill in the vertices and coordinates
  {
    GlobalOrdinal i = 0;

    for (const auto & dof : semi_local_nodes)
    {
      const Node & node = mesh.node_ref(dof);

      vertices[i] = node.id();

      for (GlobalOrdinal j = 0; j < dim; j++)
        coordinates[(j * num_local_nodes) + i] = node(j) + offset(j);

      for (GlobalOrdinal j = 0; j < from_dim; j++)
        target_coordinates[(j * num_local_nodes) + i] = node(j) + offset(j);

      i++;
    }
  }

  // Currently assuming all elements are the same!
  DataTransferKit::DTK_ElementTopology element_topology = get_element_topology(mesh.elem_ptr(0));
  GlobalOrdinal n_nodes_per_elem = mesh.elem_ptr(0)->n_nodes();

  GlobalOrdinal n_local_elem = mesh.n_local_elem();

  elements.resize(n_local_elem);
  Teuchos::ArrayRCP<GlobalOrdinal> connectivity(n_nodes_per_elem * n_local_elem);

  Teuchos::ArrayRCP<double> elem_centroid_coordinates(n_local_elem * from_dim);

  // Fill in the elements and connectivity
  {
    GlobalOrdinal i = 0;

    MeshBase::const_element_iterator end = mesh.local_elements_end();
    for (MeshBase::const_element_iterator it = mesh.local_elements_begin(); it != end; ++it)
    {
      const Elem & elem = *(*it);
      elements[i] = elem.id();

      for (GlobalOrdinal j = 0; j < n_nodes_per_elem; j++)
        connectivity[(j * n_local_elem) + i] = elem.node_id(j);

      {
        Point centroid = elem.centroid();
        for (GlobalOrdinal j = 0; j < from_dim; j++)
          elem_centroid_coordinates[(j * n_local_elem) + i] = centroid(j) + offset(j);
      }

      i++;
    }
  }

  Teuchos::ArrayRCP<int> permutation_list(n_nodes_per_elem);
  for (GlobalOrdinal i = 0; i < n_nodes_per_elem; ++i)
    permutation_list[i] = i;

  /*
  Moose::out<<"n_nodes_per_elem: "<<n_nodes_per_elem<<std::endl;

  Moose::out<<"Dim: "<<dim<<std::endl;

  Moose::err<<"Vertices size: "<<vertices.size()<<std::endl;
  {
    Moose::err<<libMesh::processor_id()<<" Vertices: ";

    for (unsigned int i=0; i<vertices.size(); i++)
      Moose::err<<vertices[i]<<" ";

    Moose::err<<std::endl;
  }

  Moose::err<<"Coordinates size: "<<coordinates.size()<<std::endl;
  {
    Moose::err<<libMesh::processor_id()<<" Coordinates: ";

    for (unsigned int i=0; i<coordinates.size(); i++)
      Moose::err<<coordinates[i]<<" ";

    Moose::err<<std::endl;
  }

  Moose::err<<"Connectivity size: "<<connectivity.size()<<std::endl;
  {
    Moose::err<<libMesh::processor_id()<<" Connectivity: ";

    for (unsigned int i=0; i<connectivity.size(); i++)
      Moose::err<<connectivity[i]<<" ";

    Moose::err<<std::endl;
  }

  Moose::err<<"Permutation_List size: "<<permutation_list.size()<<std::endl;
  {
    Moose::err<<libMesh::processor_id()<<" Permutation_List: ";

    for (unsigned int i=0; i<permutation_list.size(); i++)
      Moose::err<<permutation_list[i]<<" ";

    Moose::err<<std::endl;
  }

  */
  Teuchos::RCP<MeshContainerType> mesh_container =
      Teuchos::rcp(new MeshContainerType(dim,
                                         vertices,
                                         coordinates,
                                         element_topology,
                                         n_nodes_per_elem,
                                         elements,
                                         connectivity,
                                         permutation_list));

  // We only have 1 element topology in this grid so we make just one mesh block
  Teuchos::ArrayRCP<Teuchos::RCP<MeshContainerType>> mesh_blocks(1);
  mesh_blocks[0] = mesh_container;

  // Create the MeshManager
  mesh_manager =
      Teuchos::rcp(new DataTransferKit::MeshManager<MeshContainerType>(mesh_blocks, comm, dim));

  // Pack the coordinates into a field, this will be the positions we'll ask for other systems
  // fields at
  if (from_dim == dim)
    target_coords =
        Teuchos::rcp(new DataTransferKit::FieldManager<MeshContainerType>(mesh_container, comm));
  else
  {
    Teuchos::ArrayRCP<GlobalOrdinal> empty_elements(0);
    Teuchos::ArrayRCP<GlobalOrdinal> empty_connectivity(0);

    Teuchos::RCP<MeshContainerType> coords_only_mesh_container =
        Teuchos::rcp(new MeshContainerType(from_dim,
                                           vertices,
                                           target_coordinates,
                                           element_topology,
                                           n_nodes_per_elem,
                                           empty_elements,
                                           empty_connectivity,
                                           permutation_list));

    target_coords = Teuchos::rcp(
        new DataTransferKit::FieldManager<MeshContainerType>(coords_only_mesh_container, comm));
  }

  {
    Teuchos::ArrayRCP<GlobalOrdinal> empty_elements(0);
    Teuchos::ArrayRCP<GlobalOrdinal> empty_connectivity(0);

    Teuchos::RCP<MeshContainerType> centroid_coords_only_mesh_container =
        Teuchos::rcp(new MeshContainerType(from_dim,
                                           elements,
                                           elem_centroid_coordinates,
                                           element_topology,
                                           n_nodes_per_elem,
                                           empty_elements,
                                           empty_connectivity,
                                           permutation_list));

    elem_centroid_coords = Teuchos::rcp(new DataTransferKit::FieldManager<MeshContainerType>(
        centroid_coords_only_mesh_container, comm));
  }

  // Swap back
  Moose::swapLibMeshComm(old_comm);
}

DTKInterpolationAdapter::RCP_Evaluator
DTKInterpolationAdapter::get_variable_evaluator(std::string var_name)
{
  if (evaluators.find(var_name) ==
      evaluators.end()) // We haven't created an evaluator for the variable yet
  {
    System * sys = Transfer::find_sys(es, var_name);

    // Create the FieldEvaluator
    evaluators[var_name] = Teuchos::rcp(new DTKInterpolationEvaluator(*sys, var_name, _offset));
  }

  return evaluators[var_name];
}

Teuchos::RCP<DataTransferKit::FieldManager<DTKInterpolationAdapter::FieldContainerType>>
DTKInterpolationAdapter::get_values_to_fill(std::string var_name)
{
  if (values_to_fill.find(var_name) == values_to_fill.end())
  {
    System * sys = Transfer::find_sys(es, var_name);
    unsigned int var_num = sys->variable_number(var_name);
    bool is_nodal = sys->variable_type(var_num).family == LAGRANGE;

    Teuchos::ArrayRCP<double> data_space;

    if (is_nodal)
      data_space = Teuchos::ArrayRCP<double>(vertices.size());
    else
      data_space = Teuchos::ArrayRCP<double>(elements.size());

    Teuchos::RCP<FieldContainerType> field_container =
        Teuchos::rcp(new FieldContainerType(data_space, 1));
    values_to_fill[var_name] =
        Teuchos::rcp(new DataTransferKit::FieldManager<FieldContainerType>(field_container, comm));
  }

  return values_to_fill[var_name];
}

void
DTKInterpolationAdapter::update_variable_values(std::string var_name,
                                                Teuchos::ArrayView<GlobalOrdinal> missed_points)
{
  MPI_Comm old_comm = Moose::swapLibMeshComm(*comm->getRawMpiComm());

  System * sys = Transfer::find_sys(es, var_name);
  unsigned int var_num = sys->variable_number(var_name);

  bool is_nodal = sys->variable_type(var_num).family == LAGRANGE;

  Teuchos::RCP<FieldContainerType> values = values_to_fill[var_name]->field();

  // Create a vector containing true or false for each point saying whether it was missed or not
  // We're only going to update values for points that were not missed
  std::vector<bool> missed(values->size(), false);

  for (const auto & dof : missed_points)
    missed[dof] = true;

  unsigned int i = 0;
  // Loop over the values (one for each node) and assign the value of this variable at each node
  for (const auto & val : *values)
  {
    // If this point "missed" then skip it
    if (missed[i])
    {
      i++;
      continue;
    }

    const DofObject * dof_object = NULL;

    if (is_nodal)
      dof_object = mesh.node_ptr(vertices[i]);
    else
      dof_object = mesh.elem_ptr(elements[i]);

    if (dof_object->processor_id() == mesh.processor_id())
    {
      // The 0 is for the component... this only works for LAGRANGE!
      dof_id_type dof = dof_object->dof_number(sys->number(), var_num, 0);
      sys->solution->set(dof, val);
    }

    i++;
  }

  sys->solution->close();
  sys->update();

  // Swap back
  Moose::swapLibMeshComm(old_comm);
}

DataTransferKit::DTK_ElementTopology
DTKInterpolationAdapter::get_element_topology(const Elem * elem)
{
  ElemType type = elem->type();

  if (type == EDGE2)
    return DataTransferKit::DTK_LINE_SEGMENT;
  else if (type == EDGE3)
    return DataTransferKit::DTK_LINE_SEGMENT;
  else if (type == EDGE4)
    return DataTransferKit::DTK_LINE_SEGMENT;
  else if (type == TRI3)
    return DataTransferKit::DTK_TRIANGLE;
  else if (type == QUAD4)
    return DataTransferKit::DTK_QUADRILATERAL;
  else if (type == QUAD8)
    return DataTransferKit::DTK_QUADRILATERAL;
  else if (type == QUAD9)
    return DataTransferKit::DTK_QUADRILATERAL;
  else if (type == TET4)
    return DataTransferKit::DTK_TETRAHEDRON;
  else if (type == HEX8)
    return DataTransferKit::DTK_HEXAHEDRON;
  else if (type == HEX27)
    return DataTransferKit::DTK_HEXAHEDRON;
  else if (type == PYRAMID5)
    return DataTransferKit::DTK_PYRAMID;

  libMesh::err << "Element type not supported by DTK!" << std::endl;
  libmesh_error();
}

void
DTKInterpolationAdapter::get_semi_local_nodes(std::set<GlobalOrdinal> & semi_local_nodes)
{
  MeshBase::const_element_iterator end = mesh.local_elements_end();
  for (MeshBase::const_element_iterator it = mesh.local_elements_begin(); it != end; ++it)
  {
    const Elem & elem = *(*it);

    for (unsigned int j = 0; j < elem.n_nodes(); j++)
      semi_local_nodes.insert(elem.node_id(j));
  }
}

#endif // #ifdef LIBMESH_TRILINOS_HAVE_DTK
