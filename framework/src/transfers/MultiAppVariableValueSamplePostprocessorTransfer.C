//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppVariableValueSamplePostprocessorTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", MultiAppVariableValueSamplePostprocessorTransfer);

defineLegacyParams(MultiAppVariableValueSamplePostprocessorTransfer);

InputParameters
MultiAppVariableValueSamplePostprocessorTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Transfers the value of a variable within the master application at each sub-application "
      "position and transfers the value to a postprocessor on the sub-application(s).");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor",
      "The name of the postprocessor in the MultiApp to transfer the value to.  "
      "This should most likely be a Reciever Postprocessor.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppVariableValueSamplePostprocessorTransfer::MultiAppVariableValueSamplePostprocessorTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _postprocessor_name(getParam<PostprocessorName>("postprocessor")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppVariableValueSamplePostprocessorTransfer::execute()
{
  _console << "Beginning VariableValueSamplePostprocessorTransfer " << name() << std::endl;

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      MooseVariableField<Real> & from_var = static_cast<MooseVariableField<Real> &>(
          from_problem.getActualFieldVariable(0, _from_var_name));
      SystemBase & from_system_base = from_var.sys();
      SubProblem & from_sub_problem = from_system_base.subproblem();

      MooseMesh & from_mesh = from_problem.mesh();

      std::unique_ptr<PointLocatorBase> pl = from_mesh.getPointLocator();

      pl->enable_out_of_mesh_mode();

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        Real value = -std::numeric_limits<Real>::max();

        { // Get the value of the variable at the point where this multiapp is in the master domain

          Point multi_app_position = _multi_app->position(i);

          std::vector<Point> point_vec(1, multi_app_position);

          // First find the element the hit lands in
          const Elem * elem = (*pl)(multi_app_position);

          if (elem && elem->processor_id() == from_mesh.processor_id())
          {
            from_sub_problem.setCurrentSubdomainID(elem, 0);
            from_sub_problem.reinitElemPhys(elem, point_vec, 0);

            mooseAssert(from_var.sln().size() == 1, "No values in u!");
            value = from_var.sln()[0];
          }

          _communicator.max(value);
        }

        if (_multi_app->hasLocalApp(i))
          _multi_app->appProblemBase(i).setPostprocessorValueByName(_postprocessor_name, value);
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError(
          "Can't transfer a variable value from a MultiApp to a Postprocessor in the Master.");
      break;
    }
  }

  _console << "Finished VariableValueSamplePostprocessorTransfer " << name() << std::endl;
}
