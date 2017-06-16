/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERDATA_H
#define SAMPLERDATA_H

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerInterface.h"

class SamplerData;

template <>
InputParameters validParams<SamplerData>();

/**
 * A tool for output Sampler data.
 */
class SamplerData : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  SamplerData(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual execute() override;

  /// Storage for declared vectors
  std::vector<VectorPostprocessorValue *> _sample_vectors;

  /// The sampler to extract data
  Sampler & _sampler;
};

#endif
