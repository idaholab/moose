#pragma once

#include "AuxKernel.h"

/**
 * Platypus wrapper around VectorAuxKernel.
 *
 * Enables subclasses to call "writableVariable". This is not possible for VectorAuxKernel derived
 * classes due to a MOOSE limitation.
 */
class WritableVectorAuxKernel : public VectorAuxKernel
{
public:
  static InputParameters validParams() { return VectorAuxKernel::validParams(); }

  WritableVectorAuxKernel(const InputParameters & parameters) : VectorAuxKernel(parameters) {}

protected:
  // NB: not used.
  virtual RealVectorValue computeValue() override { mooseError("Unused"); }

  // NB: see "writableVariable" method defined in "Coupleable.h".
  MooseVariable & writableVariable(const std::string & var_name, unsigned int comp = 0);
};
