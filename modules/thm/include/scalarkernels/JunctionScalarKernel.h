#pragma once

#include "NodalScalarKernel.h"

class JunctionScalarKernel;

template <>
InputParameters validParams<JunctionScalarKernel>();

/**
 * Base class for junction constraints
 */
class JunctionScalarKernel : public NodalScalarKernel
{
public:
  JunctionScalarKernel(const InputParameters & parameters);

protected:
  std::vector<Real> _normals;
};
