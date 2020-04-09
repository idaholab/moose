//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"
#include "SamplerBase.h"

// Forward declarations
class EulerAngleProvider;

/**
 *  GrainTextureVectorPostprocessor is a VectorPostprocessor that outputs the
 *  the coordinates, grain number, and Euler Angles associated with each element.
 *  Currently only works with a uniform, structured grid (no mesh adaptivity).
 */
class GrainTextureVectorPostprocessor : public ElementVectorPostprocessor, protected SamplerBase
{
public:
  static InputParameters validParams();

  GrainTextureVectorPostprocessor(const InputParameters & parameters);
  virtual void initialize();
  virtual void execute();
  using SamplerBase::threadJoin;
  virtual void threadJoin(const UserObject & uo);
  virtual void finalize();

protected:
  const EulerAngleProvider & _euler;
  const VariableValue & _unique_grains;
  const unsigned int _grain_num;
  std::vector<Real> _sample;
};
