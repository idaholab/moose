//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPostprocessorTransfer.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", MultiAppPostprocessorTransfer);

InputParameters
MultiAppPostprocessorTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Transfers postprocessor data between the master application and sub-application(s).");
  params.addRequiredParam<PostprocessorName>(
      "from_postprocessor",
      "The name of the Postprocessor in the Master to transfer the value from.");
  params.addRequiredParam<PostprocessorName>(
      "to_postprocessor",
      "The name of the Postprocessor in the MultiApp to transfer the value to. "
      " This should most likely be a Reporter Postprocessor.");
  MooseEnum reduction_type("average sum maximum minimum");
  params.addParam<MooseEnum>("reduction_type",
                             reduction_type,
                             "The type of reduction to perform to reduce postprocessor "
                             "values from multiple SubApps to a single value");
  return params;
}

MultiAppPostprocessorTransfer::MultiAppPostprocessorTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_pp_name(getParam<PostprocessorName>("from_postprocessor")),
    _to_pp_name(getParam<PostprocessorName>("to_postprocessor")),
    _reduction_type(getParam<MooseEnum>("reduction_type"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");

  if (_current_direction == FROM_MULTIAPP)
    if (!_reduction_type.isValid())
      mooseError("In MultiAppPostprocessorTransfer, must specify 'reduction_type' if direction = "
                 "from_multiapp");

  if (isParamValid("to_multi_app") && isParamValid("from_multi_app") &&
      isParamValid("reduction_type"))
    mooseError("Reductions are not supported for multiapp sibling transfers");
}

void
MultiAppPostprocessorTransfer::execute()
{
  TIME_SECTION("MultiAppPostprocessorTransfer::execute()", 5, "Transferring a postprocessor");

  switch (_current_direction)
  {
    case BETWEEN_MULTIAPP:
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      {
        if (getFromMultiApp()->hasLocalApp(i))
        {
          // Get source postprocessor value
          FEProblemBase & from_problem = getFromMultiApp()->appProblemBase(i);
          const Real & pp_value = from_problem.getPostprocessorValueByName(_from_pp_name);

          if (getToMultiApp()->hasLocalApp(i))
            getToMultiApp()->appProblemBase(i).setPostprocessorValueByName(_to_pp_name, pp_value);
        }
      }
      break;
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = getToMultiApp()->problemBase();

      const Real & pp_value = from_problem.getPostprocessorValueByName(_from_pp_name);

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
        if (getToMultiApp()->hasLocalApp(i))
          getToMultiApp()->appProblemBase(i).setPostprocessorValueByName(_to_pp_name, pp_value);
      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblemBase & to_problem = getFromMultiApp()->problemBase();

      Real reduced_pp_value;
      switch (_reduction_type)
      {
        case AVERAGE:
        case SUM:
          reduced_pp_value = 0;
          break;
        case MAXIMUM:
          reduced_pp_value = -std::numeric_limits<Real>::max();
          break;
        case MINIMUM:
          reduced_pp_value = std::numeric_limits<Real>::max();
          break;
        default:
          mooseError(
              "Can't get here unless someone adds a new enum and fails to add it to this switch");
      }

      const auto multi_app = getFromMultiApp() ? getFromMultiApp() : getToMultiApp();

      for (unsigned int i = 0; i < multi_app->numGlobalApps(); i++)
      {
        if (multi_app->hasLocalApp(i) && multi_app->isRootProcessor())
        {
          const Real & curr_pp_value =
              multi_app->appProblemBase(i).getPostprocessorValueByName(_from_pp_name);
          switch (_reduction_type)
          {
            case AVERAGE:
            case SUM:
              reduced_pp_value += curr_pp_value;
              break;
            case MAXIMUM:
              reduced_pp_value = std::max(curr_pp_value, reduced_pp_value);
              break;
            case MINIMUM:
              reduced_pp_value = std::min(curr_pp_value, reduced_pp_value);
              break;
            default:
              mooseError("Can't get here unless someone adds a new enum and fails to add it to "
                         "this switch");
          }
        }
      }

      switch (_reduction_type)
      {
        case AVERAGE:
          _communicator.sum(reduced_pp_value);
          reduced_pp_value /= static_cast<Real>(multi_app->numGlobalApps());
          break;
        case SUM:
          _communicator.sum(reduced_pp_value);
          break;
        case MAXIMUM:
          _communicator.max(reduced_pp_value);
          break;
        case MINIMUM:
          _communicator.min(reduced_pp_value);
          break;
        default:
          mooseError(
              "Can't get here unless someone adds a new enum and fails to add it to this switch");
      }

      to_problem.setPostprocessorValueByName(_to_pp_name, reduced_pp_value);
      break;
    }
  }
}
