//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDEVALUESAMPLER_H
#define SIDEVALUESAMPLER_H

#include "SideVectorPostprocessor.h"
#include "SamplerBase.h"

// Forward Declarations
class SideValueSampler;

template <>
InputParameters validParams<SideValueSampler>();

class SideValueSampler : public SideVectorPostprocessor, protected SamplerBase
{
public:
  SideValueSampler(const InputParameters & parameters);

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

  unsigned int _qp;
};

#endif
