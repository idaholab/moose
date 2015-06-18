#include "ICEUpdater.h"
#include <sstream>
#include "FEProblem.h"
#include "PostprocessorWarehouse.h"
#include "Postprocessor.h"

template<>
InputParameters validParams<ICEUpdater>() {

	InputParameters params = validParams<AdvancedOutput<Output> >();
	params += AdvancedOutput<Output>::enableOutputTypes("postprocessor");

	params.addRequiredParam<std::string>("item_id",
			"The currently running MOOSE Item Id.");
	params.addRequiredParam<std::string>("url",
			"The URL of the currently running ICE Core instance.");

	return params;
}

ICEUpdater::ICEUpdater(const std::string & name, InputParameters parameters) :
		AdvancedOutput<Output>(name, parameters) {

	// Create the iStream containing the initialization data for the Updater
	std::stringstream ss;
	ss << "item_id=" + getParam<std::string>("item_id") + "\n";
	ss << "client_key=1234567890ABCDEFGHIJ1234567890ABCDEFGHIJ\n";
	ss << "url=" << getParam<std::string>("url") + "\n";
	ss << "username=ice\n";
	ss << "password=veryice\n";

	// Create the ICE Updater
	iceUpdater = MooseSharedPointer<Updater>(new Updater(ss));

	// Start the Updater.
	iceUpdater->start();
}

ICEUpdater::~ICEUpdater() {
	iceUpdater->stop();
}

void ICEUpdater::initialSetup() {
	// Call base class setup method
	AdvancedOutput<Output>::initialSetup();

	// The libMesh::ExodusII_IO will fail when it is closed if the object is created but
	// nothing is written to the file. This checks that at least something will be written.
	if (!hasOutput())
		mooseError(
				"The current settings result in nothing being output to the file.");

	// Test that some sort of variable output exists (case when all variables are disabled but input output is still enabled
	if (!hasNodalVariableOutput() && !hasElementalVariableOutput()
			&& !hasPostprocessorOutput() && !hasScalarOutput())
		mooseError(
				"The current settings results in only the input file and no variables being output to the file, this is not supported.");
}

void ICEUpdater::outputPostprocessors() {
	// List of desired postprocessor outputs
	const std::set<std::string> & pps = getPostprocessorOutput();
	for (std::set<std::string>::const_iterator it = pps.begin();
			it != pps.end(); ++it) {
		PostprocessorValue value = _problem_ptr->getPostprocessorValue(*it);
		std::stringstream ss;
		ss << *it << ":" << _time << ":" << value;
		iceUpdater->postMessage(ss.str());
	}
}
