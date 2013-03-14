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

#include "MultiAppVariableValueSamplePostprocessorTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppVariableValueSamplePostprocessorTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor in the MultiApp to transfer the value to.  This should most likely be a Reporter Postprocessor.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppVariableValueSamplePostprocessorTransfer::MultiAppVariableValueSamplePostprocessorTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _postprocessor_name(getParam<PostprocessorName>("postprocessor")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
}

void
MultiAppVariableValueSamplePostprocessorTransfer::execute()
{
  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
      SystemBase & from_system_base = from_var.sys();
      SubProblem & from_sub_problem = from_system_base.subproblem();

      MooseMesh & from_mesh = from_problem.mesh();

      AutoPtr<PointLocatorBase> pl = from_mesh.getMesh().sub_point_locator();

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        Real value = -std::numeric_limits<Real>::max();

        { // Get the value of the variable at the point where this multiapp is in the master domain

          Point multi_app_position = _multi_app->position(i);

          std::vector<Point> point_vec(1, multi_app_position);

          // First find the element the hit lands in
          const Elem * elem = (*pl)(multi_app_position);

          if(elem && elem->processor_id() == libMesh::processor_id())
          {
            from_sub_problem.reinitElemPhys(elem, point_vec, 0);

            mooseAssert(from_var.sln().size() == 1, "No values in u!");
            value = from_var.sln()[0];
          }

          libMesh::Parallel::max(value);
        }

        if(_multi_app->hasLocalApp(i))
          _multi_app->appProblem(i)->getPostprocessorValue(_postprocessor_name) = value;
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError("Can't transfer a variable value from a MultiApp to a Postprocessor in the Master.");
      break;
    }
  }
}
