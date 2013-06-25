#ifndef PPSDIFFUSION_H
#define PPSDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class PPSDiffusion;

template<>
InputParameters validParams<PPSDiffusion>();


class PPSDiffusion : public Kernel
{
public:
  PPSDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  PostprocessorValue & _pps_value;
};

#endif //PPSDIFFUSION_H
