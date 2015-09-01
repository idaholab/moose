/*
 * ICEUpdater.h
 *
 *  Created on: Jun 12, 2015
 *      Author: aqw
 */

#ifndef ICE_UPDATER_H
#define ICE_UPDATER_H

// MOOSE includes
#include "AdvancedOutput.h"
#include "Output.h"
#include "InputParameters.h"
#include "Updater.h"

// Forward declarations
class ICEUpdater;

template<>
InputParameters validParams<ICEUpdater>();

/**
 */
class ICEUpdater: public AdvancedOutput<Output> {
public:

	ICEUpdater(const InputParameters & params);

	virtual ~ICEUpdater();

	/**
	 * Performs basic error checking and initial setup of ExodusII_IO output object
	 */
	virtual void initialSetup();

protected:

	/**
	 * Writes postprocessor values to global output parameters
	 */
	virtual void outputPostprocessors();

private:

	MooseSharedPointer<Updater> iceUpdater;

};

#endif
