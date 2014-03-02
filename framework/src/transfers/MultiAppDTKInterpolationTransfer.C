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

#include "MultiAppDTKInterpolationTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

template<>
InputParameters validParams<MultiAppDTKInterpolationTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppDTKInterpolationTransfer::MultiAppDTKInterpolationTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _from_var_name(getParam<VariableName>("source_variable")),
    _to_var_name(getParam<AuxVariableName>("variable"))
{
}

void
MultiAppDTKInterpolationTransfer::execute()
{
  // Every processor has to be involved with every transfer because the "master" domain is on all processors
  // However, each processor might or might not have the particular multiapp on it.  When that happens
  // we need to pass NULL in for the variable and the MPI_Comm for that piece of the transfer
  for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
  {
    switch(_direction)
    {
      case TO_MULTIAPP:
      {
        System * from_sys = find_sys(_multi_app->problem()->es(), _from_var_name);
        System * to_sys = NULL;

        if (_multi_app->hasLocalApp(i))
          to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

        _helper.transferWithOffset(0, i, &from_sys->variable(from_sys->variable_number(_from_var_name)),
                                   to_sys ? &to_sys->variable(to_sys->variable_number(_to_var_name)) : NULL,
                                   _master_position, _multi_app->position(i),
                                   &libMesh::COMM_WORLD, to_sys ? &_multi_app->comm() : NULL);
        break;
      }

      case FROM_MULTIAPP:
      {
        System * from_sys = NULL;
        System * to_sys = find_sys(_multi_app->problem()->es(), _to_var_name);

        if (_multi_app->hasLocalApp(i))
          from_sys = find_sys(_multi_app->appProblem(i)->es(), _from_var_name);

        _helper.transferWithOffset(i, 0, from_sys ? &from_sys->variable(from_sys->variable_number(_from_var_name)) : NULL,
                                   &to_sys->variable(to_sys->variable_number(_to_var_name)),
                                   _multi_app->position(i), _master_position,
                                   from_sys ? &_multi_app->comm() : NULL, &libMesh::COMM_WORLD);
        break;
      }

      _multi_app->problem()->es().update();

      if (_multi_app->hasLocalApp(i))
        _multi_app->appProblem(i)->es().update();
    }
  }
}

#endif //LIBMESH_HAVE_DTK
