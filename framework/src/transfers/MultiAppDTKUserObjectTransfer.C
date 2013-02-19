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

#include "MultiAppDTKUserObjectTransfer.h"

#include "MooseTypes.h"

template<>
InputParameters validParams<MultiAppDTKUserObjectTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<UserObjectName>("user_object", "The UserObject you want to transfer values from.  Note: This might be a UserObject from your MultiApp's input file!");
  return params;
}

MultiAppDTKUserObjectTransfer::MultiAppDTKUserObjectTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    MooseVariableInterface(parameters, true),
    _user_object_name(getParam<UserObjectName>("user_object")),
    _setup(false)
{
}

void
MultiAppDTKUserObjectTransfer::execute()
{
  if(!_setup)
  {
    _setup = true;

    _comm_default = Teuchos::DefaultComm<int>::getComm();

    _multi_app_user_object_evaluator = Teuchos::rcp(new MultiAppDTKUserObjectEvaluator(*_multi_app, _user_object_name));

    _field_evaluator = _multi_app_user_object_evaluator;

    _multi_app_geom = _multi_app_user_object_evaluator->createSourceGeometry(_comm_default);

    _to_adapter = new DTKAdapter(_comm_default, _multi_app->problem()->es());

    _src_to_tgt_map = new DataTransferKit::VolumeSourceMap<DataTransferKit::Box, int, DataTransferKit::MeshContainer<int> >(_comm_default, 3, false);

    std::cout<<"--Setting Up Transfer--"<<std::endl;
    _src_to_tgt_map->setup(_multi_app_geom, _to_adapter->get_target_coords());
    std::cout<<"--Transfer Setup Complete--"<<std::endl;

    _to_values = _to_adapter->get_values_to_fill(_variable->name());

  }

  std::cout<<"--Mapping Values--"<<std::endl;
  _src_to_tgt_map->apply(_field_evaluator, _to_values);
  std::cout<<"--Finished Mapping--"<<std::endl;

  _to_adapter->update_variable_values(_variable->name());
  _multi_app->problem()->es().update();
}

#endif //LIBMESH_HAVE_DTK
