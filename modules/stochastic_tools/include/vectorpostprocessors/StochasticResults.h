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

  /**
   * Initialize storage based on the Sampler returned by the SamplerMultiApp.
   * @param sampler The Sampler associated with the MultiApp that this VPP is working with.
   *
   * This method is called by the SamplerPostprocessorTransfer.
   */
  void init(Sampler & _sampler);

  /**
   * Return the VectorPostprocessorValue for a given Sampler group index.
   * @param group Index related to the index of the DenseMatrix returned by Sampler::getSamples()
   * @return A reference to the storage location for the PP data from the sub-applications.
   */
  VectorPostprocessorValue & getVectorPostprocessorValueByGroup(unsigned int group);

  /**
   * Get the sample vectors
   * @return A const pointer to the vector of sample vectors
   */
  const std::vector<VectorPostprocessorValue *> & getSampleVectors() const
  {
    return _sample_vectors;
  }

protected:
  /// Storage for declared vectors
  std::vector<VectorPostprocessorValue *> _sample_vectors;

  /// The sampler to extract data
  Sampler * _sampler = nullptr;
};

#endif
