//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SamplerParameterTransfer.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerFullSolveMultiApp.h"
#include "SamplerReceiver.h"
#include "Sampler.h"
#include "MooseUtils.h"
#include "pcrecpp.h"

registerMooseObjectRenamed("StochasticToolsApp",
                           SamplerTransfer,
                           "01/01/2020 00:00",
                           SamplerParameterTransfer);
registerMooseObject("StochasticToolsApp", SamplerParameterTransfer);

InputParameters
SamplerParameterTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Copies Sampler data to a SamplerReceiver object.");
  params.suppressParameter<MultiMooseEnum>("direction");
  params.suppressParameter<MultiAppName>("multi_app");
  params.addParam<std::vector<std::string>>(
      "parameters",
      "A list of parameters (on the sub application) to control "
      "with the Sampler data. The order of the parameters listed "
      "here should match the order of the items in the Sampler.");
  params.addDeprecatedParam<std::string>(
      "to_control",
      "Unused parameter.",
      "This parameter is no longer used as the SamplerReceiver will automatically be detected.");
  return params;
}

SamplerParameterTransfer::SamplerParameterTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _parameter_names(getParam<std::vector<std::string>>("parameters"))
{
  if (hasFromMultiApp())
    paramError("from_multi_app", "From and between multiapp directions are not implemented");
}

void
SamplerParameterTransfer::execute()
{
  mooseAssert((_sampler_ptr->getNumberOfLocalRows() == 0) ||
                  (_sampler_ptr->getNumberOfLocalRows() == getToMultiApp()->numLocalApps()),
              "The number of MultiApps and the number of sample rows must be the same.");

  // Loop over all sub-apps
  for (dof_id_type row_index = _sampler_ptr->getLocalRowBegin();
       row_index < _sampler_ptr->getLocalRowEnd();
       row_index++)
  {
    mooseAssert(getToMultiApp()->hasLocalApp(row_index),
                "The current sample row index is not a valid global MultiApp index.");

    // Populate the row of data to transfer
    std::vector<Real> row = _sampler_ptr->getNextLocalRow();

    // Get the command line arguments
    const auto args = SamplerFullSolveMultiApp::sampledCommandLineArgs(row, _parameter_names);

    // Get the sub-app SamplerReceiver objects and transfer param-values map
    for (auto & ptr : getReceivers(row_index, args))
      ptr.first->transfer(ptr.second);
  }
}

void
SamplerParameterTransfer::executeToMultiapp()
{
  if (getToMultiApp()->isRootProcessor())
  {
    // Get the command line arguments
    const auto args = SamplerFullSolveMultiApp::sampledCommandLineArgs(_row_data, _parameter_names);

    // Get the sub-app SamplerReceiver objects and transfer param-values map
    for (auto & ptr : getReceivers(_app_index, args))
      ptr.first->transfer(ptr.second);
  }
}

std::map<SamplerReceiver *, std::map<std::string, std::vector<Real>>>
SamplerParameterTransfer::getReceivers(unsigned int app_index,
                                       const std::vector<std::string> & args)
{
  // These are the receivers we are returning
  std::map<SamplerReceiver *, std::map<std::string, std::vector<Real>>> ptrs;

  // Split all the input parameters to retrieve receivers and set param-values pairs
  // Cache the multiapp-reciever mapping
  std::unordered_map<std::string, std::vector<SamplerReceiver *>> multiapp_receivers;
  for (const auto & param : args)
  {
    // Get MultiApp Name
    std::string multiapp_name = getToMultiApp()->name();
    const auto pos = param.rfind(":");
    if (pos != std::string::npos)
      multiapp_name += ":" + param.substr(0, pos);
    // Get parameter name
    const auto pos2 = param.rfind("=");
    if (pos2 == std::string::npos)
      mooseError("Internal error: Improper command-line format\n   ", param, ".");
    const std::string param_name = param.substr(pos + 1, pos2 - pos - 1);

    // Get values
    const std::string vstr = param.substr(pos2 + 1);
    const auto vpos1 = vstr.find("'");
    const auto vpos2 = vstr.rfind("'");
    std::vector<Real> value;
    if (!MooseUtils::tokenizeAndConvert(vstr.substr(vpos1 + 1, vpos2 - vpos1 - 1), value, " "))
      mooseError("Internal error: Improper command-line format\n   ", param, ".");

    // Find receivers if we haven't already found them
    if (multiapp_receivers.find(multiapp_name) == multiapp_receivers.end())
    {
      auto & recvs = multiapp_receivers[multiapp_name]; // Convenience
      // Split the <sub>:<subsub>:<subsubsub> syntax
      const std::vector<std::string> nested_names = MooseUtils::split(multiapp_name, ":");
      // Find all the appropriate FEProblemBases based on the parameter syntax and get a
      // SamplerReceiver
      for (const auto & problem : getMultiAppProblemsHelper(
               getToMultiApp()->appProblemBase(app_index),
               std::vector<std::string>(nested_names.begin() + 1, nested_names.end())))
      {
        // Loop through all the active controls and find one that is a SamplerReciever
        SamplerReceiver * recv_ptr = nullptr;
        for (const auto & control_ptr : problem->getControlWarehouse().getActiveObjects())
        {
          recv_ptr = dynamic_cast<SamplerReceiver *>(control_ptr.get());
          if (recv_ptr)
            break;
        }
        if (!recv_ptr)
          paramError("parameters",
                     "The sub-application (",
                     multiapp_name,
                     ") does not contain a SamplerReceiver control object.");

        // Insert the found receiver
        recvs.push_back(recv_ptr);
      }
    }

    // Add the parameter and value to the reciever map
    for (const auto & recv_ptr : multiapp_receivers[multiapp_name])
      ptrs[recv_ptr][param_name] = value;
  }

  return ptrs;
}

std::vector<FEProblemBase *>
SamplerParameterTransfer::getMultiAppProblemsHelper(FEProblemBase & base_problem,
                                                    const std::vector<std::string> & multiapp_names)
{
  // This is what we'll return
  std::vector<FEProblemBase *> problems;

  if (multiapp_names.empty()) // Stop recursion if no nested apps provided
    problems.push_back(&base_problem);
  else
  {
    // Get the name of highest level app provided and the index if provided
    std::string sub_name;
    int sub_num = -1;
    pcrecpp::RE("(\\S*?)(\\d*)").FullMatch(multiapp_names[0], &sub_name, &sub_num);
    MultiApp & multiapp = *base_problem.getMultiApp(sub_name);

    // Get the app indices (either the requested one or all of them)
    std::pair<unsigned int, unsigned int> app_ind;
    if (sub_num == -1) // There was no number provided, so get all the local sub apps
      app_ind = std::make_pair(multiapp.firstLocalApp(),
                               multiapp.firstLocalApp() + multiapp.numLocalApps());
    else if (multiapp.hasLocalApp(sub_num)) // Number provided, get that app if owned
      app_ind = std::make_pair(sub_num, sub_num + 1);
    else // This processor doesn't own the app index provided
      return {};

    // Get all the nested subapps with recursion
    std::vector<std::string> nested_names(multiapp_names.begin() + 1, multiapp_names.end());
    for (unsigned int i = app_ind.first; i < app_ind.second; ++i)
      for (auto & prob : getMultiAppProblemsHelper(multiapp.appProblemBase(i), nested_names))
        problems.push_back(prob);
  }

  return problems;
}
