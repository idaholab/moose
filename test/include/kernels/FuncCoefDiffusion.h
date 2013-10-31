#ifndef FUNCCOEFDIFFUSION_H
#define FUNCCOEFDIFFUSION_H

// MOOSE includes
#include "Kernel.h"
#include "Function.h"

//Forward Declarations
class FuncCoefDiffusion;

template<>
InputParameters validParams<FuncCoefDiffusion>();

/**
 * A kernel for testing the MooseParsedFunctionInterface
 */
class FuncCoefDiffusion :
  public Kernel
{
public:
  FuncCoefDiffusion(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Function &  _function;
};

#endif //FUNCCOEFDIFFUSION_H
