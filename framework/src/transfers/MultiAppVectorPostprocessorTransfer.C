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

InputParameters
MultiAppVectorPostprocessorTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
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
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppVectorPostprocessorTransfer::executeToMultiapp()
{
  const VectorPostprocessorValue & vpp =
      getToMultiApp()->problemBase().getVectorPostprocessorValueByName(_master_vpp_name,
                                                                       _vector_name);

  if (vpp.size() != getToMultiApp()->numGlobalApps())
    mooseError("VectorPostprocessor ",
               _master_vpp_name,
               " and number of sub-apps do not match: ",
               vpp.size(),
               "/",
               getToMultiApp()->numGlobalApps());

  for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); ++i)
    if (getToMultiApp()->hasLocalApp(i))
      getToMultiApp()->appProblemBase(i).setPostprocessorValueByName(_sub_pp_name, vpp[i]);
}

void
MultiAppVectorPostprocessorTransfer::executeFromMultiapp()
{
  const VectorPostprocessorValue & vpp =
      getFromMultiApp()->problemBase().getVectorPostprocessorValueByName(_master_vpp_name,
                                                                         _vector_name);

  if (vpp.size() != getFromMultiApp()->numGlobalApps())
    mooseError("VectorPostprocessor ",
               _master_vpp_name,
               " and number of sub-apps do not match: ",
               vpp.size(),
               "/",
               getFromMultiApp()->numGlobalApps());

  VectorPostprocessorValue value(getFromMultiApp()->numGlobalApps(), 0.0);
  for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); ++i)
    if (getFromMultiApp()->hasLocalApp(i))
      value[i] = getFromMultiApp()->appProblemBase(i).getPostprocessorValueByName(_sub_pp_name);

  for (auto & v : value)
    _communicator.sum(v);

  getFromMultiApp()->problemBase().setVectorPostprocessorValueByName(
      _master_vpp_name, _vector_name, value);
}

void
MultiAppVectorPostprocessorTransfer::execute()
{
  TIME_SECTION(
      "MultiAppVectorPostprocessorTransfer::execute()", 5, "Transferring a vector postprocessor");

  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
}
