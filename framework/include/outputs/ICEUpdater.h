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

#ifndef ICE_UPDATER_H
#define ICE_UPDATER_H

// MOOSE includes
#include "AdvancedOutput.h"
#include "Output.h"
#include "InputParameters.h"

// Forward declarations
class Updater;

// Currently the ICE Updater requires C++11 threads and std::condition_variable
#if defined(LIBMESH_HAVE_CXX11_THREAD) && defined(LIBMESH_HAVE_CXX11_CONDITION_VARIABLE)

// Forward declarations
class ICEUpdater;

template<>
InputParameters validParams<ICEUpdater>();

/**
 * The ICEUpdater is a subclass of AdvancedOutput<Output> that
 * provides the functionality to post Postprocessor data at each
 * time step over a web socket back to the Eclipse Integrated
 * Computational Environment (ICE) for real-time plot updates.
 */
class ICEUpdater: public AdvancedOutput<Output>
{
public:
  /**
   * The Constructor
   */
  ICEUpdater(const InputParameters & params);

  virtual ~ICEUpdater();

  /**
   * Performs basic error checking and initial setup
   */
  virtual void initialSetup() override;

protected:
  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors() override;

  /**
   * Reference to the ICE Updater object in charge of
   * connecting to ICE and posting update messages.
   */
  MooseSharedPointer<Updater> _updater;

  /**
   * If true, set (CURLOPT_NOPROXY, "*") when calling libcurl APIs.
   */
  bool _noproxy;
};

#endif

#endif
