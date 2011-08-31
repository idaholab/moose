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

  std::string _pps_name;
  PostprocessorValue & _pps_value;
};

#endif //PPSDIFFUSION_H
