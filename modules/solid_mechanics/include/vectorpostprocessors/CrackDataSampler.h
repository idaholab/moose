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

#ifndef CRACK_DATA_SAMPLER_H
#define CRACK_DATA_SAMPLER_H

#include "GeneralVectorPostprocessor.h"
#include "CrackFrontDefinition.h"
#include "SamplerBase.h"

//Forward Declarations
class CrackDataSampler;

template<>
InputParameters validParams<CrackDataSampler>();

/**
 *  CrackDataSampler is a type of VectorPostprocessor that outputs the values of
 *  domain integrals, printed along with positions and angles along the crack front
 */

class CrackDataSampler :
  public GeneralVectorPostprocessor,
  public SamplerBase
{
public:
  /**
    * Class constructor
    * @param name The name of the object
    * @param parameters The input parameters
    */
  CrackDataSampler(const std::string & name, InputParameters parameters);

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
  virtual void threadJoin(const SamplerBase &);

protected:
  const CrackFrontDefinition * const _crack_front_definition;

  MooseEnum  _position_type;

  /// The vector of PostprocessorValue objects that are used to get the values of the domain integral postprocessors
  std::vector<const PostprocessorValue *> _domain_integral_postprocessor_values;
};

#endif
