#ifndef CONVECTIONPRECOMPUTE_H
#define CONVECTIONPRECOMPUTE_H

#include "KernelValue.h"

// Forward Declarations
class ConvectionPrecompute;

template<>
InputParameters validParams<ConvectionPrecompute>();

class ConvectionPrecompute : public KernelValue
{
public:

  ConvectionPrecompute(const std::string & name,
             InputParameters parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();

private:
  RealVectorValue _velocity;
};

#endif // CONVECTIONPRECOMPUTE_H
