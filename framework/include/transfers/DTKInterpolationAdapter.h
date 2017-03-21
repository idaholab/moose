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

#ifndef DTKINTERPOLATIONADAPTER_H
#define DTKINTERPOLATIONADAPTER_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

// libMesh includes
#include "libmesh/point.h"

// Ignore warnings coming from Trilinos/DTK.
#include "libmesh/ignore_warnings.h"

// DTK includes
#include <DTK_MeshManager.hpp>
#include <DTK_MeshContainer.hpp>
#include <DTK_MeshTraits.hpp>
#include <DTK_MeshTraitsFieldAdapter.hpp>
#include <DTK_FieldManager.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_FieldEvaluator.hpp>

// Trilinos includes
#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_DefaultMpiComm.hpp>

// Restore warnings.
#include "libmesh/restore_warnings.h"

// Forward declarations
namespace libMesh
{
class EquationSystems;
class Elem;
class MeshBase;
class System;
}

class DTKInterpolationAdapter
{
public:
  DTKInterpolationAdapter(Teuchos::RCP<const Teuchos::MpiComm<int>> in_comm,
                          libMesh::EquationSystems & in_es,
                          const libMesh::Point & offset,
                          unsigned int from_dim);

  typedef DataTransferKit::MeshContainer<long unsigned int> MeshContainerType;
  typedef DataTransferKit::FieldContainer<double> FieldContainerType;
  typedef DataTransferKit::MeshTraits<MeshContainerType>::global_ordinal_type GlobalOrdinal;
  typedef DataTransferKit::FieldEvaluator<GlobalOrdinal, FieldContainerType> EvaluatorType;
  typedef Teuchos::RCP<EvaluatorType> RCP_Evaluator;

  Teuchos::RCP<DataTransferKit::MeshManager<MeshContainerType>> get_mesh_manager()
  {
    return mesh_manager;
  }
  RCP_Evaluator get_variable_evaluator(std::string var_name);
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType>> get_target_coords()
  {
    return target_coords;
  }

  /**
   * Used to get the centroids for the receiving elements.
   */
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType>> get_elem_target_coords()
  {
    return elem_centroid_coords;
  }

  Teuchos::RCP<DataTransferKit::FieldManager<FieldContainerType>>
  get_values_to_fill(std::string var_name);

  /**
   * After computing values for a variable in this EquationSystems
   * we need to take those values and put them in the actual solution vector.
   */
  void update_variable_values(std::string var_name,
                              Teuchos::ArrayView<GlobalOrdinal> missed_points);

protected:
  /**
   * Helper that returns the DTK ElementTopology for a given Elem
   */
  DataTransferKit::DTK_ElementTopology get_element_topology(const libMesh::Elem * elem);

  /**
   * Helper function that fills the std::set with all of the node numbers of
   * nodes connected to local elements.
   */
  void get_semi_local_nodes(std::set<GlobalOrdinal> & semi_local_nodes);

  Teuchos::RCP<const Teuchos::MpiComm<int>> comm;
  libMesh::EquationSystems & es;
  libMesh::Point _offset;
  const libMesh::MeshBase & mesh;
  unsigned int dim;

  unsigned int num_local_nodes;
  Teuchos::ArrayRCP<GlobalOrdinal> vertices;
  Teuchos::ArrayRCP<GlobalOrdinal> elements;

  Teuchos::RCP<DataTransferKit::MeshManager<MeshContainerType>> mesh_manager;
  RCP_Evaluator field_evaluator;
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType>> target_coords;
  Teuchos::RCP<DataTransferKit::FieldManager<MeshContainerType>> elem_centroid_coords;

  /// Map of variable names to arrays to be filled by a transfer
  std::map<std::string, Teuchos::RCP<DataTransferKit::FieldManager<FieldContainerType>>>
      values_to_fill;

  /// Map of variable names to RCP_Evaluator objects
  std::map<std::string, RCP_Evaluator> evaluators;
};

#endif // #ifdef LIBMESH_TRILINOS_HAVE_DTK

#endif // #define DTKINTERPOLATIONADAPTER_H
