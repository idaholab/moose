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

#ifndef LINEFUNCTIONSAMPLER_H
#define LINEFUNCTIONSAMPLER_H

#include "GeneralVectorPostprocessor.h"
#include "SamplerBase.h"
#include "Function.h"

// Forward Declarations
class LineFunctionSampler;

template <>
InputParameters validParams<LineFunctionSampler>();

class LineFunctionSampler : public GeneralVectorPostprocessor, protected SamplerBase
{
public:
  LineFunctionSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  // Let the SamplerBase version of threadJoin() take part in the
  // overload resolution process, otherwise we get warnings about
  // overloaded virtual functions and "hiding" in debug mode.
  using SamplerBase::threadJoin;

  // TODO: Is this even called (threadJoin on a general object)?
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Beginning of the line
  Point _start_point;

  /// End of the line
  Point _end_point;

  /// Number of points along the line
  unsigned int _num_points;

  /// Names of the Functions
  const std::vector<FunctionName> & _function_names;

  /// Number of Functions we're sampling
  unsigned int _num_funcs;

  /// Pointers to the Functions
  std::vector<Function *> _functions;

  /// So we don't have to create and destroy this vector over and over again
  std::vector<Real> _values;

  /// The points to evaluate at
  std::vector<Point> _points;

  /// The ID to use for each point (yes, this is Real on purpose)
  std::vector<Real> _ids;
};

#endif
