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
#include <sstream>
#include "FEProblem.h"
#include "PostprocessorWarehouse.h"
#include "Postprocessor.h"

template<>
InputParameters validParams<ICEUpdater>()
{
  InputParameters params = validParams<AdvancedOutput<Output> >();
  params += AdvancedOutput<Output>::enableOutputTypes("postprocessor");

  params.addRequiredParam<std::string>("item_id", "The currently running MOOSE Item Id.");
  params.addRequiredParam<std::string>("url", "The URL of the currently running ICE Core instance.");
  params.addParam<bool>("noproxy", true, "If true, set 'CURLOPT_NOPROXY, \"*\"' when calling libcurl APIs.");

  return params;
}

ICEUpdater::ICEUpdater(const InputParameters & parameters) :
    AdvancedOutput<Output>(parameters),
    _noproxy(getParam<bool>("noproxy"))
{
  // Create the iStream containing the initialization data for the Updater
  std::stringstream ss;
  ss << "item_id=" + getParam<std::string>("item_id") + "\n";
  ss << "client_key=1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ\n";
  ss << "url=" << getParam<std::string>("url") + "\n";
  ss << "username=ice\n";
  ss << "password=veryice\n";

  // Create the ICE Updater
  iceUpdater = MooseSharedPointer<Updater>(new Updater(ss));

  // Pass down the _noproxy flag to the iceUpdater.
  iceUpdater->setNoProxyFlag(_noproxy);

  // Start the Updater.
  iceUpdater->start();
}

ICEUpdater::~ICEUpdater()
{
  // Stop the ICEUpdater thread
  iceUpdater->stop();
}

void ICEUpdater::initialSetup()
{
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
    iceUpdater->postMessage(ss.str());
  }
}
