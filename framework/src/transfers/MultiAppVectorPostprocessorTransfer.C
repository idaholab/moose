//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppVectorPostprocessorTransfer.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", MultiAppVectorPostprocessorTransfer);

template <>
InputParameters
validParams<MultiAppVectorPostprocessorTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the Postprocessors on the sub-app to transfer from/to.");
  params.addRequiredParam<VectorPostprocessorName>("vector_postprocessor",
                                                   "The name of the VectorPostprocessor in "
                                                   "the MultiApp to transfer values "
                                                   "from/to.");
  params.addRequiredParam<std::string>(
      "vector_name", "Named vector quantity to transfer from/to in VectorPostprocessor.");
  params.addClassDescription("This transfer distributes the N values of a "
                             "VectorPostprocessor to Postprocessors located in "
                             "N sub-apps or"
                             " collects Postprocessor values from N sub-apps "
                             "into a VectorPostprocessor");
  return params;
}

MultiAppVectorPostprocessorTransfer::MultiAppVectorPostprocessorTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _sub_pp_name(getParam<PostprocessorName>("postprocessor")),
    _master_vpp_name(getParam<VectorPostprocessorName>("vector_postprocessor")),
    _vector_name(getParam<std::string>("vector_name"))
{
}

void
MultiAppVectorPostprocessorTransfer::executeToMultiapp()
{
  VectorPostprocessorValue & vpp =
      _multi_app->problemBase().getVectorPostprocessorValue(_master_vpp_name, _vector_name, false);

  if (vpp.size() != _multi_app->numGlobalApps())
    mooseError("VectorPostprocessor ",
               _master_vpp_name,
               " and number of sub-apps do not match: ",
               vpp.size(),
               "/",
               _multi_app->numGlobalApps());

  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
      _multi_app->appProblemBase(i).getPostprocessorValue(_sub_pp_name) = vpp[i];
}

void
MultiAppVectorPostprocessorTransfer::executeFromMultiapp()
{
  VectorPostprocessorValue & vpp =
      _multi_app->problemBase().getVectorPostprocessorValue(_master_vpp_name, _vector_name, false);

  if (vpp.size() != _multi_app->numGlobalApps())
    mooseError("VectorPostprocessor ",
               _master_vpp_name,
               " and number of sub-apps do not match: ",
               vpp.size(),
               "/",
               _multi_app->numGlobalApps());

  // set all values to zero to make communication easier
  for (auto & v : vpp)
    v = 0.0;

  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
      vpp[i] = _multi_app->appProblemBase(i).getPostprocessorValue(_sub_pp_name);

  for (auto & v : vpp)
    _communicator.sum(v);
}

void
MultiAppVectorPostprocessorTransfer::execute()
{
  _console << "Beginning VectorPostprocessorTransfer " << name() << std::endl;

  if (_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();

  _console << "Finished PostprocessorTransfer " << name() << std::endl;
}
