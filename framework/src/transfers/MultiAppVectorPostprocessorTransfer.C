//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  // Execute VPP if it was specified to execute on transfers
  _fe_problem.computeUserObjectByName(EXEC_TRANSFER, Moose::PRE_AUX, _master_vpp_name);
  _fe_problem.computeUserObjectByName(EXEC_TRANSFER, Moose::POST_AUX, _master_vpp_name);

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
  errorIfObjectExecutesOnTransferInSourceApp(_sub_pp_name);

  if (vpp.size() != getFromMultiApp()->numGlobalApps())
    mooseError("VectorPostprocessor ",
               _master_vpp_name,
               " and number of sub-apps do not match: ",
               vpp.size(),
               "/",
               getFromMultiApp()->numGlobalApps());

  VectorPostprocessorValue value(getFromMultiApp()->numGlobalApps(), 0.0);
  for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); ++i)
  {
    // To avoid duplication and thus a wrong pp value after summing value accross procs, we should
    // ensure that value[i] is only set by one procs. To do this, we check isFirstLocalRank to see
    // if the current proc is the first rank that subapp i lives on.
    if (getFromMultiApp()->hasLocalApp(i) && getFromMultiApp()->isFirstLocalRank())
      value[i] = getFromMultiApp()->appProblemBase(i).getPostprocessorValueByName(_sub_pp_name);
  }

  // Sum to distribute entries of 'value' accross all procs
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
