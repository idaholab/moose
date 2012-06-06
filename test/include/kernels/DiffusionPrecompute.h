#ifndef DIFFUSIONPRECOMPUTE_H
#define DIFFUSIONPRECOMPUTE_H

#include "KernelGrad.h"

class DiffusionPrecompute;

template<>
InputParameters validParams<DiffusionPrecompute>();


class DiffusionPrecompute : public KernelGrad
{
public:
  DiffusionPrecompute(const std::string & name, InputParameters parameters);
  virtual ~DiffusionPrecompute();

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
};


#endif /* DIFFUSIONPRECOMPUTE_H */
