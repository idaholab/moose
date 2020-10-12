//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppVppToVppTransfer.h"
#include "VectorPostprocessorReceiver.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"

registerMooseObject("isopodApp", MultiAppVppToVppTransfer);

InputParameters
MultiAppVppToVppTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();

  params.addClassDescription("This transfer copies all of the VectorPostprocessor values on the \n"
                             "master app to each sub-app and vice versa.");

  params.addRequiredParam<VectorPostprocessorName>("vector_postprocessor_sub",
                                                   "The name of the VectorPostprocessor in "
                                                   "the master to transfer values "
                                                   "from/to.");

  params.addRequiredParam<VectorPostprocessorName>("vector_postprocessor_master",
                                                   "The name of the VectorPostprocessor in "
                                                   "the master to transfer values "
                                                   "from/to.");

  return params;
}

MultiAppVppToVppTransfer::MultiAppVppToVppTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _sub_vpp_name(getParam<VectorPostprocessorName>("vector_postprocessor_sub")),
    _master_vpp_name(getParam<VectorPostprocessorName>("vector_postprocessor_master"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppVppToVppTransfer::initialSetup()
{
  _console << "Beginning VppToVppTransfer initial setup" << name() << std::endl;

  if (_current_direction == FROM_MULTIAPP)
    initialSetupFromMultiapp();
  else
    initialSetupToMultiapp();

  _console << "Finished VppToVppTransfer initial setup" << name() << std::endl;
}

void
MultiAppVppToVppTransfer::execute()
{
  _console << "Beginning VppToVppTransfer " << name() << std::endl;

  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else if (_current_direction == TO_MULTIAPP)
    executeToMultiapp();

  _console << "Finished VppToVppTransfer " << name() << std::endl;
}

void
MultiAppVppToVppTransfer::executeFromMultiapp()
{
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & fe_base = _multi_app->appProblemBase(i);
      copyVectorPostprocessors(fe_base, _sub_vpp_name);
    }
}

void
MultiAppVppToVppTransfer::executeToMultiapp()
{
  FEProblemBase & fe_base = _multi_app->problemBase();
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
      copyVectorPostprocessors(fe_base, _master_vpp_name);
}

void
MultiAppVppToVppTransfer::copyVectorPostprocessors(FEProblemBase & fe_base,
                                                   const VectorPostprocessorName & vpp_name)
{
  const auto & vpp_vector = fe_base.getVectorPostprocessorVectors(vpp_name);

  mooseAssert(_receiver_vpps.size() == vpp_vector.size(),
              "MultiAppVppToVppTransfer::executeToMultiapp(): mismatched number of columns "
              "in receiver and sender");

  for (size_t j = 0; j < _receiver_vpps.size(); ++j)
  {
    std::string vector_name = vpp_vector[j].first;
    const VectorPostprocessorValue & vpp_value =
        fe_base.getVectorPostprocessorValue(vpp_name, vector_name, false);
    (*_receiver_vpps[j]) = vpp_value;
  }
}

void
MultiAppVppToVppTransfer::initialSetupFromMultiapp()
{
  // intializing master_vpp from sub_vpp

  std::cout << "initialSetupFromMultiapp numGlobalApps= " << _multi_app->numGlobalApps()
            << std::endl;

  auto & master_vpp =
      _multi_app->problemBase().getUserObject<VectorPostprocessorReceiver>(_master_vpp_name);

  _receiver_vpps.clear();
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
  {
    if (_multi_app->hasLocalApp(i))
    {
      const auto & sub_vpp_vectors =
          _multi_app->appProblemBase(i).getVectorPostprocessorVectors(_sub_vpp_name);

      _receiver_vpps.resize(sub_vpp_vectors.size());
      unsigned int index = 0;
      for (const auto & sub_vpp_vector : sub_vpp_vectors)
      {
        _receiver_vpps[index] = &(master_vpp.addVector(sub_vpp_vector.first));
        ++index;
      }
    }
  }
}

void
MultiAppVppToVppTransfer::initialSetupToMultiapp()
{
  // intializing sub_vpp from master_vpp

  std::cout << "initialSetupToMultiapp numGlobalApps= " << _multi_app->numGlobalApps() << std::endl;

  const auto & master_vpp_vectors =
      _multi_app->problemBase().getVectorPostprocessorVectors(_master_vpp_name);

  _receiver_vpps.clear();
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
  {
    if (_multi_app->hasLocalApp(i))
    {
      auto & sub_vpp =
          _multi_app->appProblemBase(i).getUserObject<VectorPostprocessorReceiver>(_sub_vpp_name);

      _receiver_vpps.resize(master_vpp_vectors.size());
      unsigned int index = 0;
      for (const auto & master_vpp_vector : master_vpp_vectors)
      {
        _receiver_vpps[index] = &(sub_vpp.addVector(master_vpp_vector.first));
        ++index;
      }
    }
  }
}
