/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINTEXTUREVECTORPOSTPROCESSOR
#define GRAINTEXTUREVECTORPOSTPROCESSOR

#include "ElementVectorPostprocessor.h"
#include "SamplerBase.h"

// Forward declarations
class EulerAngleProvider;
class GrainTextureVectorPostprocessor;

template <>
InputParameters validParams<GrainTextureVectorPostprocessor>();

/**
 *  GrainTextureVectorPostprocessor is a VectorPostprocessor that outputs the
 *  the coordinates, grain number, and Euler Angles associated with each element.
 *  Currently only works with a uniform, structured grid (no mesh adaptivity).
 */
class GrainTextureVectorPostprocessor : public ElementVectorPostprocessor, protected SamplerBase
{
public:
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

#endif // GRAINTEXTUREVECTORPOSTPROCESSOR_H
