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

#ifndef NODALVALUESAMPLER_H
#define NODALVALUESAMPLER_H

#include "NodalVariableVectorPostprocessor.h"
#include "SamplerBase.h"

// Forward Declarations
class NodalValueSampler;

template <>
InputParameters validParams<NodalValueSampler>();

class NodalValueSampler : public NodalVariableVectorPostprocessor, protected SamplerBase
{
public:
  NodalValueSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  // Let the SamplerBase version of threadJoin() take part in the
  // overload resolution process, otherwise we get warnings about
  // overloaded virtual functions and "hiding" in debug mode.
  using SamplerBase::threadJoin;

  virtual void threadJoin(const UserObject & y) override;

protected:
  /// So we don't have to create and destroy this vector over and over again
  std::vector<Real> _values;

  /// Vector of 0 and 1 values which records whether values are present at the current node.
  std::vector<unsigned int> _has_values;
};

#endif
