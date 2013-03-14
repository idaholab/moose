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

#ifdef LIBMESH_HAVE_DTK

#include "MoabTransfer.h"

#include "FEProblem.h"

#include "libmesh/dtk_adapter.h"

#include <DTK_MeshManager.hpp>
#include <DTK_MeshTools.hpp>
#include <DTK_SharedDomainMap.hpp>
#include <DTK_FieldEvaluator.hpp>
#include <DTK_MeshTraits.hpp>
#include <DTK_MeshTraitsFieldAdapter.hpp>

#include "MoabMesh.h"
#include "ArrayField.h"
#include "MoabEvaluator.h"

#include <Teuchos_GlobalMPISession.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_OpaqueWrapper.hpp>
#include <Teuchos_TypeTraits.hpp>

template<>
InputParameters validParams<MoabTransfer>()
{
  InputParameters params = validParams<Transfer>();

  MooseEnum direction_options("from_moab, to_moab");
  params.addRequiredParam<MooseEnum>("direction", direction_options, "Whether this Transfer will be 'to' or 'from' a MultiApp.");

  return params;
}

MoabTransfer::MoabTransfer(const std::string & name, InputParameters parameters) :
    Transfer(name, parameters),
    _direction(getParam<MooseEnum>("direction"))
{
}

void
MoabTransfer::execute()
{
  if((int)_direction == 0)
  {
    Teuchos::RCP<const Teuchos::Comm<int> > comm_default = Teuchos::DefaultComm<int>::getComm();
    EquationSystems * to_es = &_fe_problem.es();
    DTKAdapter * moose_adapter = new DTKAdapter(comm_default, *to_es);

    DataTransferKit::SharedDomainMap<MoabMesh::Container,DTKAdapter::MeshContainerType> * map =
      new DataTransferKit::SharedDomainMap<MoabMesh::Container,DTKAdapter::MeshContainerType>(comm_default, to_es->get_mesh().mesh_dimension(), true);

    int mesh_dim = to_es->get_mesh().mesh_dimension();
    MoabMesh source_mesh(comm_default, "heat_source.vtk", moab::MBHEX, 0 );
    Teuchos::ArrayRCP<Teuchos::RCP<MoabMesh::Container> > source_blocks( 1, source_mesh.meshContainer() );
    Teuchos::RCP<DataTransferKit::MeshManager<MoabMesh::Container> > source_mesh_manager = Teuchos::rcp( new DataTransferKit::MeshManager<MoabMesh::Container>( source_blocks, comm_default, mesh_dim ) );

    map->setup(source_mesh_manager, moose_adapter->get_target_coords(), 100*Teuchos::ScalarTraits<double>::eps());

    Teuchos::RCP<DataTransferKit::FieldEvaluator<MoabMesh::Container::global_ordinal_type,ArrayField> > moab_evaluator = Teuchos::rcp( new MoabEvaluator( source_mesh, "heat_source" ) );

    Teuchos::RCP<DataTransferKit::FieldManager<DTKAdapter::FieldContainerType> > to_values = moose_adapter->get_values_to_fill("heat_source");

    map->apply(moab_evaluator, to_values);

    moose_adapter->update_variable_values("heat_source");

    delete moose_adapter;
    delete map;
  }
  else
  {
    Teuchos::RCP<const Teuchos::Comm<int> > comm_default = Teuchos::DefaultComm<int>::getComm();
    EquationSystems * from_es = &_fe_problem.es();

    DTKAdapter * moose_adapter = new DTKAdapter(comm_default, *from_es);

    DataTransferKit::SharedDomainMap<DTKAdapter::MeshContainerType,MoabMesh::Container> * map =
      new DataTransferKit::SharedDomainMap<DTKAdapter::MeshContainerType,MoabMesh::Container>(comm_default, from_es->get_mesh().mesh_dimension(), true);

    MoabMesh target_mesh( comm_default, "heat_source.vtk", moab::MBHEX, 0);
    Teuchos::RCP<DataTransferKit::FieldManager<MoabMesh::Container> > target_coords_manager = Teuchos::rcp( new DataTransferKit::FieldManager<MoabMesh::Container>(target_mesh.meshContainer(), comm_default ) );

    map->setup(moose_adapter->get_mesh_manager(), target_coords_manager, 100*Teuchos::ScalarTraits<double>::eps());

    DTKAdapter::RCP_Evaluator from_evaluator = moose_adapter->get_variable_evaluator("rod_temp");

    int num_targets = DataTransferKit::MeshTools<MoabMesh::Container>::numVertices(*(target_mesh.meshContainer()) );

    Teuchos::RCP<ArrayField> data_target = Teuchos::rcp( new ArrayField( num_targets, 1 ) );
    Teuchos::RCP<DataTransferKit::FieldManager<ArrayField> > target_space_manager = Teuchos::rcp( new DataTransferKit::FieldManager<ArrayField>(data_target, comm_default ) );

    map->apply(from_evaluator, target_space_manager);

    target_mesh.tag("rod_temp", *data_target);
    target_mesh.write( "moab_out.vtk" );

    delete moose_adapter;
    delete map;
  }

}

#endif //LIBMESH_HAVE_DTK
