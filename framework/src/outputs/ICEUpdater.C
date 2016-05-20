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

#include "ICEUpdater.h"
#include "FEProblem.h"
#include "Postprocessor.h"
#include "Updater.h"
#include <sstream>
#include "MooseTypes.h"

// Currently the ICE Updater requires std::thread and std::condition_variable.
#if defined(LIBMESH_HAVE_CXX11_THREAD) && defined(LIBMESH_HAVE_CXX11_CONDITION_VARIABLE)

template<>
InputParameters validParams<ICEUpdater>()
{
  InputParameters params = validParams<AdvancedOutput<Output> >();
  params += AdvancedOutput<Output>::enableOutputTypes("postprocessor");

  // Get an MooseEnum of the available networkingTools
  MooseEnum networkingTools("asio=0 curl=1");

  params.addParam<MooseEnum>("networkingTool", networkingTools, "The back-end networking tool to use - could be asio, curl, etc...");
  params.addRequiredParam<std::string>("item_id", "The currently running MOOSE Item Id.");
  params.addRequiredParam<std::string>("url", "The URL of the currently running ICE Core instance.");
  params.addParam<bool>("noproxy", true, "If true, set 'CURLOPT_NOPROXY, \"*\"' when calling libcurl APIs.");

  params.set<MooseEnum>("networkingTool") = "asio";

  return params;
}

ICEUpdater::ICEUpdater(const InputParameters & parameters) :
    AdvancedOutput<Output>(parameters),
    _noproxy(getParam<bool>("noproxy"))
{
  if (_communicator.rank() != 0)
    return;

  // Create the iStream containing the initialization data for the Updater
  std::stringstream ss;
  ss << "item_id=" + getParam<std::string>("item_id") + "\n";
  ss << "client_key=1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ\n";
  ss << "url=" << getParam<std::string>("url") + "\n";
  ss << "username=ice\n";
  ss << "password=veryice\n";
  ss << "networkingTool=" + std::string(getParam<MooseEnum>("networkingTool")) + "\n";

  // Create the ICE Updater
  _updater = MooseSharedPointer<Updater>(new Updater(ss));

  // Pass down the _noproxy flag to the iceUpdater.
  _updater->setNoProxyFlag(_noproxy);

  // Start the Updater.
  _updater->start();

}

ICEUpdater::~ICEUpdater()
{
  if (_communicator.rank() != 0)
    return;

  // Stop the ICEUpdater thread
  _updater->stop();
}

void ICEUpdater::initialSetup()
{
  if (_communicator.rank() != 0)
    return;

  // Call base class setup method
  AdvancedOutput<Output>::initialSetup();

  // The libMesh::ExodusII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the file.");

  // Test that some sort of variable output exists (case when all variables are disabled but input output is still enabled
  if (!hasNodalVariableOutput()     &&
      !hasElementalVariableOutput() &&
      !hasPostprocessorOutput()     &&
      !hasScalarOutput())
    mooseError("The current settings results in only the input file and no variables being output to the file, this is not supported.");
}

void ICEUpdater::outputPostprocessors()
{
  if (_communicator.rank() != 0)
    return;

  // Get the list of Postprocessors
  const std::set<std::string> & pps = getPostprocessorOutput();
  for (std::set<std::string>::const_iterator it = pps.begin(); it != pps.end(); ++it)
  {

    // Grab the value at the current time
    PostprocessorValue value = _problem_ptr->getPostprocessorValue(*it);

    // Create a string stream to use in posting the message
    std::stringstream ss;

    // Create the message as PPName:time:value
    ss << *it << ":" << _time << ":" << value;

    // Post the message
    _updater->postMessage(ss.str());
  }
}

#endif
