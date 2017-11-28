/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERPOSTPROCESSORTRANSFER_H
#define SAMPLERPOSTPROCESSORTRANSFER_H

// MOOSE includes
#include "MultiAppVectorPostprocessorTransfer.h"
#include "Sampler.h"

// Forward declarations
class SamplerPostprocessorTransfer;
class SamplerReceiver;
class SamplerMultiApp;
class StochasticResults;

template <>
InputParameters validParams<SamplerPostprocessorTransfer>();

/**
 * Transfer Postprocessor from sub-applications to the master application.
 */
class SamplerPostprocessorTransfer : public MultiAppVectorPostprocessorTransfer
{
public:
  SamplerPostprocessorTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual void executeFromMultiapp() override;

  /// SamplerMultiApp that this transfer is working with
  SamplerMultiApp * _sampler_multi_app;

  /// Sampler object that is retrieved from the SamplerMultiApp
  Sampler & _sampler;

  /// Storage for StochasticResults object that data will be transferred to/from
  StochasticResults * _results;
};

#endif
