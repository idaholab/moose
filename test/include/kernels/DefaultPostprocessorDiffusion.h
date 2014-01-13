#ifndef DEFAULTPOSTPROCESSORDIFFUSION_H
#define DEFAULTPOSTPROCESSORDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class DefaultPostprocessorDiffusion;

template<>
InputParameters validParams<DefaultPostprocessorDiffusion>();


class DefaultPostprocessorDiffusion : public Kernel
{
public:
  DefaultPostprocessorDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const PostprocessorValue & _pps_value;
};

#endif //DEFAULTPOSTPROCESSORDIFFUSION_H
