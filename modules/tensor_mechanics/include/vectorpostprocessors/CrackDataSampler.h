/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CRACK_DATA_SAMPLER_H
#define CRACK_DATA_SAMPLER_H

#include "GeneralVectorPostprocessor.h"
#include "CrackFrontDefinition.h"
#include "SamplerBase.h"

// Forward Declarations
class CrackDataSampler;

template <>
InputParameters validParams<CrackDataSampler>();

/**
 *  CrackDataSampler is a type of VectorPostprocessor that outputs the values of
 *  domain integrals, printed along with positions and angles along the crack front
 */

class CrackDataSampler : public GeneralVectorPostprocessor, public SamplerBase
{
public:
  /**
    * Class constructor
    * @param parameters The input parameters
    */
  CrackDataSampler(const InputParameters & parameters);

  /**
   * Destructor
   */
  virtual ~CrackDataSampler() {}

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize();

  /**
   * Populates the postprocessor vector of values with the supplied postprocessors
   */
  virtual void execute();

  virtual void finalize();

protected:
  const CrackFrontDefinition * const _crack_front_definition;

  MooseEnum _position_type;

  /// The vector of PostprocessorValue objects that are used to get the values of the domain integral postprocessors
  std::vector<const PostprocessorValue *> _domain_integral_postprocessor_values;
};

#endif
