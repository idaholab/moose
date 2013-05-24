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

// Moose Includes
#include "MooseError.h"

#include "DTKInterpolationHelper.h"

#include "libmesh/system.h"

// Trilinos Includes
#include <Teuchos_RCP.hpp>
#include <Teuchos_GlobalMPISession.hpp>
#include <Teuchos_Ptr.hpp>
#include <Teuchos_OpaqueWrapper.hpp>

// DTK Includes
#include <DTK_MeshManager.hpp>
#include <DTK_MeshContainer.hpp>
#include <DTK_FieldManager.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_FieldTools.hpp>
#include <DTK_CommTools.hpp>
#include <DTK_CommIndexer.hpp>

namespace libMesh {

DTKInterpolationHelper::DTKInterpolationHelper()
{}

DTKInterpolationHelper::~DTKInterpolationHelper()
{
  for(std::map<EquationSystems *, DTKInterpolationAdapter *>::iterator it = adapters.begin();
      it != adapters.end();
      ++it)
    delete it->second;

  for(std::map<std::pair<unsigned int, unsigned int>, shared_domain_map_type * >::iterator it = dtk_maps.begin();
      it != dtk_maps.end();
      ++it)
    delete it->second;
}

void
DTKInterpolationHelper::transferWithOffset(unsigned int from, unsigned int to, const Variable * from_var, const Variable * to_var, const Point & from_offset, const Point & to_offset, MPI_Comm * from_mpi_comm, MPI_Comm * to_mpi_comm)
{
  Teuchos::RCP<const Teuchos::MpiComm<int> > from_comm;
  Teuchos::RCP<const Teuchos::MpiComm<int> > to_comm;

  if(from_mpi_comm)
    from_comm = Teuchos::rcp(new Teuchos::MpiComm<int>(Teuchos::rcp(new Teuchos::OpaqueWrapper<MPI_Comm>(*from_mpi_comm))));

  if(to_mpi_comm)
    to_comm = Teuchos::rcp(new Teuchos::MpiComm<int>(Teuchos::rcp(new Teuchos::OpaqueWrapper<MPI_Comm>(*to_mpi_comm))));

  // Create a union comm for the shared domain transfer
  Teuchos::RCP<const Teuchos::Comm<int> > comm_union;

  DataTransferKit::CommTools::unite( from_comm, to_comm, comm_union );

  EquationSystems * from_es = NULL;
  EquationSystems * to_es = NULL;

  if(from_mpi_comm)
    from_es = &from_var->system()->get_equation_systems();

  if(to_mpi_comm)
    to_es = &to_var->system()->get_equation_systems();

  unsigned int dim = 3;

  if(from_es && to_es && (from_es->get_mesh().mesh_dimension() < to_es->get_mesh().mesh_dimension()))
    mooseError("Receiving system dimension should be less than or equal to the sending system dimension!");

  if(from_es && to_es)
    dim = std::max(from_es->get_mesh().mesh_dimension(), to_es->get_mesh().mesh_dimension());
  else if(from_es)
    dim = from_es->get_mesh().mesh_dimension();
  else
    dim = to_es->get_mesh().mesh_dimension();

  // Possibly make an Adapter for from_es
  if(from_mpi_comm && adapters.find(from_es) == adapters.end())
    adapters[from_es] = new DTKInterpolationAdapter(from_comm, *from_es, from_offset, dim);

  // Possibly make an Adapter for to_es
  if(to_mpi_comm && adapters.find(to_es) == adapters.end())
    adapters[to_es] = new DTKInterpolationAdapter(to_comm, *to_es, to_offset, dim);

  DTKInterpolationAdapter * from_adapter = NULL;
  DTKInterpolationAdapter * to_adapter = NULL;

  if(from_mpi_comm)
    from_adapter = adapters[from_es];

  if(to_mpi_comm)
    to_adapter = adapters[to_es];

  std::pair<int, int> from_to(from, to);

  // If we haven't created a map for this pair of EquationSystems yet, do it now
  if(dtk_maps.find(from_to) == dtk_maps.end())
  {
    shared_domain_map_type * map = NULL;

    if(from_mpi_comm)
      map = new shared_domain_map_type(comm_union, from_es->get_mesh().mesh_dimension(), true);
    else if(to_mpi_comm)
      map = new shared_domain_map_type(comm_union, to_es->get_mesh().mesh_dimension(), true);

    dtk_maps[from_to] = map;

    // The tolerance here is for the "contains_point()" implementation in DTK.  Set a larger value for a looser tolerance...
    if(from_mpi_comm && to_mpi_comm)
    {
      if(to_var->type() == FEType())
        map->setup(from_adapter->get_mesh_manager(), to_adapter->get_target_coords(), 200*Teuchos::ScalarTraits<double>::eps());
      else
        map->setup(from_adapter->get_mesh_manager(), to_adapter->get_elem_target_coords(), 200*Teuchos::ScalarTraits<double>::eps());
    }
    else if(from_mpi_comm)
      map->setup(from_adapter->get_mesh_manager(),
                 Teuchos::RCP<DataTransferKit::FieldManager<DTKInterpolationAdapter::MeshContainerType> >(),
                 200*Teuchos::ScalarTraits<double>::eps());
    else if(to_mpi_comm)
    {
      if(to_var->type() == FEType())
        map->setup(Teuchos::RCP<DataTransferKit::MeshManager<DTKInterpolationAdapter::MeshContainerType> >(),
                   to_adapter->get_target_coords(),
                   200*Teuchos::ScalarTraits<double>::eps());
      else
        map->setup(Teuchos::RCP<DataTransferKit::MeshManager<DTKInterpolationAdapter::MeshContainerType> >(),
                   to_adapter->get_elem_target_coords(),
                   200*Teuchos::ScalarTraits<double>::eps());
    }
  }

  DTKInterpolationAdapter::RCP_Evaluator from_evaluator;

  if(from_mpi_comm)
    from_evaluator = from_adapter->get_variable_evaluator(from_var->name());

  Teuchos::RCP<DataTransferKit::FieldManager<DTKInterpolationAdapter::FieldContainerType> > to_values;

  if(to_mpi_comm)
    to_values = to_adapter->get_values_to_fill(to_var->name());

  dtk_maps[from_to]->apply(from_evaluator, to_values);

  if(to_mpi_comm)
    to_adapter->update_variable_values(to_var->name(), dtk_maps[from_to]->getMissedTargetPoints());
}

} // namespace libMesh

#endif // #ifdef LIBMESH_HAVE_DTK
