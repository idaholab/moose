#pragma once

#include "ODEKernel.h"

class PostprocessorSourceScalarKernel;

template <>
InputParameters validParams<PostprocessorSourceScalarKernel>();

/**
 * Adds an arbitrary post-processor value as a source term
 */
class PostprocessorSourceScalarKernel : public ODEKernel
{
public:
  PostprocessorSourceScalarKernel(const InputParameters & params);

  virtual Real computeQpResidual() override;

protected:
  /// Post-processor to act as source
  const PostprocessorValue & _pp;
};
