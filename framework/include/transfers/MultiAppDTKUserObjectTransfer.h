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

#ifdef LIBMESH_HAVE_DTK

#ifndef MULTIAPPDTKUSEROBJECTTRANSFER_H
#define MULTIAPPDTKUSEROBJECTTRANSFER_H

#include "MultiAppTransfer.h"
#include "MooseVariableInterface.h"
#include "MultiAppDTKUserObjectEvaluator.h"

// DTK Includes
#include <DTK_VolumeSourceMap.hpp>
#include <DTK_MeshManager.hpp>
#include <DTK_MeshContainer.hpp>
#include <DTK_MeshTypes.hpp>
#include <DTK_MeshTraitsFieldAdapter.hpp>
#include <DTK_FieldEvaluator.hpp>
#include <DTK_FieldManager.hpp>
#include <DTK_FieldContainer.hpp>
#include <DTK_FieldTools.hpp>
#include <DTK_CommTools.hpp>
#include <DTK_GeometryManager.hpp>
#include <DTK_Box.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_ArrayRCP.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_GlobalMPISession.hpp>
#include <Teuchos_Ptr.hpp>


// libMesh Includes
#include "libmesh/dtk_solution_transfer.h"

class MooseVariable;
class MultiAppDTKUserObjectTransfer;

template<>
InputParameters validParams<MultiAppDTKUserObjectTransfer>();

/**
 * Transfers from spatially varying UserObjects in a MultiApp to the "master" system.
 */
class MultiAppDTKUserObjectTransfer :
  public MultiAppTransfer,
  public MooseVariableInterface
{
public:
  MultiAppDTKUserObjectTransfer(const std::string & name, InputParameters parameters);
  virtual ~MultiAppDTKUserObjectTransfer() {}

  virtual void execute();

protected:
  std::string _user_object_name;

  bool _setup;

  Teuchos::RCP<const Teuchos::Comm<int> > _comm_default;

  Teuchos::RCP<MultiAppDTKUserObjectEvaluator> _multi_app_user_object_evaluator;

  Teuchos::RCP<DataTransferKit::FieldEvaluator<int, DataTransferKit::FieldContainer<double> > > _field_evaluator;

  Teuchos::RCP<DataTransferKit::GeometryManager<DataTransferKit::Box,int> > _multi_app_geom;

  DTKAdapter * _to_adapter;

  DataTransferKit::VolumeSourceMap<DataTransferKit::Box, int, DataTransferKit::MeshContainer<int> > * _src_to_tgt_map;

  Teuchos::RCP<DataTransferKit::FieldManager<DTKAdapter::FieldContainerType> > _to_values;
};

#endif /* MULTIAPPDTKUSEROBJECTTRANSFER_H */

#endif //LIBMESH_HAVE_DTK
