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



#ifndef DTKINTERPOLATIONADAPTER_H
#define DTKINTERPOLATIONADAPTER_H

#include "Moose.h"

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "libmesh/dtk_evaluator.h"

#include <DTK_MeshManager.hpp>
#include <DTK_MeshContainer.hpp>
#include <DTK_MeshTraits.hpp>
#include <DTK_MeshTraitsFieldAdapter.hpp>
#include <DTK_FieldManager.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_FieldEvaluator.hpp>

#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_DefaultMpiComm.hpp>

class DTKInterpolationAdapter
{
public:
  DTKInterpolationAdapter(Teuchos::RCP<const Teuchos::MpiComm<int> > in_comm, EquationSystems & in_es, const Point & offset, unsigned int from_dim);

  typedef DataTransferKit::MeshContainer<int>                                  MeshContainerType;
  typedef DataTransferKit::FieldContainer<double>                              FieldContainerType;
  typedef DataTransferKit::MeshTraits<MeshContainerType>::global_ordinal_type  GlobalOrdinal;
  typedef DataTransferKit::FieldEvaluator<GlobalOrdinal,FieldContainerType>    EvaluatorType;
  typedef Teuchos::RCP<EvaluatorType>                                          RCP_Evaluator;


  Teuchos::RCP<DataTransferKit::MeshManager<MeshContainerType> > get_mesh_manager() { return mesh_manager; }
  RCP_Evaluator get_variable_evaluator(std::string var_name);
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType> > get_target_coords() { return target_coords; }

  /**
   * Used to get the centroids for the receiving elements.
   */
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType> > get_elem_target_coords() { return elem_centroid_coords; }

  Teuchos::RCP<DataTransferKit::FieldManager<FieldContainerType> > get_values_to_fill(std::string var_name);

  /**
   * After computing values for a variable in this EquationSystems
   * we need to take those values and put them in the actual solution vector.
   */
  void update_variable_values(std::string var_name, Teuchos::ArrayView<int> missed_points);

protected:
  /**
   * Small helper function for finding the system containing the variable.
   *
   * Note that this implies that variable names are unique across all systems!
   */
  System * find_sys(std::string var_name);

  /**
   * Helper that returns the DTK ElementTopology for a given Elem
   */
  DataTransferKit::DTK_ElementTopology get_element_topology(const Elem * elem);

  /**
   * Helper function that fills the std::set with all of the node numbers of
   * nodes connected to local elements.
   */
  void get_semi_local_nodes(std::set<unsigned int> & semi_local_nodes);

  Teuchos::RCP<const Teuchos::MpiComm<int> > comm;
  EquationSystems & es;
  Point _offset;
  const MeshBase & mesh;
  unsigned int dim;

  unsigned int num_local_nodes;
  Teuchos::ArrayRCP<int> vertices;
  Teuchos::ArrayRCP<int> elements;

  Teuchos::RCP<DataTransferKit::MeshManager<MeshContainerType> > mesh_manager;
  RCP_Evaluator field_evaluator;
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType> > target_coords;
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType> > elem_centroid_coords;

  /// Map of variable names to arrays to be filled by a transfer
  std::map<std::string, Teuchos::RCP<DataTransferKit::FieldManager<FieldContainerType> > > values_to_fill;

  /// Map of variable names to RCP_Evaluator objects
  std::map<std::string, RCP_Evaluator> evaluators;
};

#endif // #ifdef LIBMESH_HAVE_DTK

#endif // #define DTKINTERPOLATIONADAPTER_H
