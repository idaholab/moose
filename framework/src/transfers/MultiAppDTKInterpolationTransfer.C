//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

// MOOSE includes
#include "MultiAppDTKInterpolationTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", MultiAppDTKInterpolationTransfer);

template <>
InputParameters
validParams<MultiAppDTKInterpolationTransfer>()
{
  InputParameters params = validParams<MultiAppFieldTransferInterface>();
  return params;
}

MultiAppDTKInterpolationTransfer::MultiAppDTKInterpolationTransfer(
    const InputParameters & parameters)
  : MultiAppFieldTransferInterface(parameters)
{
}

void
MultiAppDTKInterpolationTransfer::execute()
{
  // Every processor has to be involved with every transfer because the "master" domain is on all
  // processors
  // However, each processor might or might not have the particular multiapp on it.  When that
  // happens
  // we need to pass NULL in for the variable and the MPI_Comm for that piece of the transfer
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
  {
    switch (_direction)
    {
      case TO_MULTIAPP:
      {
        System * from_sys = find_sys(_multi_app->problemBase().es(), _from_var_name);
        System * to_sys = NULL;

        if (_multi_app->hasLocalApp(i))
          to_sys = find_sys(_multi_app->appProblemBase(i).es(), _to_var_name);

        _helper.transferWithOffset(
            0,
            i,
            &from_sys->variable(from_sys->variable_number(_from_var_name)),
            to_sys ? &to_sys->variable(to_sys->variable_number(_to_var_name)) : NULL,
            _master_position,
            _multi_app->position(i),
            const_cast<libMesh::Parallel::communicator *>(&_communicator.get()),
            to_sys ? &_multi_app->comm() : NULL);
        break;
      }

      case FROM_MULTIAPP:
      {
        System * from_sys = NULL;
        System * to_sys = find_sys(_multi_app->problemBase().es(), _to_var_name);

        if (_multi_app->hasLocalApp(i))
          from_sys = find_sys(_multi_app->appProblemBase(i).es(), _from_var_name);

        _helper.transferWithOffset(
            i,
            0,
            from_sys ? &from_sys->variable(from_sys->variable_number(_from_var_name)) : NULL,
            &to_sys->variable(to_sys->variable_number(_to_var_name)),
            _multi_app->position(i),
            _master_position,
            from_sys ? &_multi_app->comm() : NULL,
            const_cast<libMesh::Parallel::communicator *>(&_communicator.get()));
        break;
      }

        _multi_app->problemBase().es().update();

        if (_multi_app->hasLocalApp(i))
          _multi_app->appProblemBase(i).es().update();
    }
  }

  postExecute();
}

#endif // LIBMESH_TRILINOS_HAVE_DTK
