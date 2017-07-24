/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STOCHASTICRESULTS_H
#define STOCHASTICRESULTS_H

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerInterface.h"

class StochasticResults;

template <>
InputParameters validParams<StochasticResults>();

/**
 * A tool for output Sampler data.
 */
class StochasticResults : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  StochasticResults(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual finalize() override {}
  void virtual execute() override {}

protected:
  /// Initialize storage based on this
  void init(Sampler & _sampler);

  /// Return the VectorPostprocessorValue for a given Sampler group index
  VectorPostprocessorValue & getVectorPostprocessorValueByGroup(unsigned int group);

  /// Storage for declared vectors
  std::vector<VectorPostprocessorValue *> _sample_vectors;

  /// The sampler to extract data
  Sampler * _sampler = nullptr;

  /// This object is designed to work with the SamplerPostprocessorTransfer, the init and
  /// get methods should only be called by this class.
  friend class SamplerPostprocessorTransfer;
};

#endif
