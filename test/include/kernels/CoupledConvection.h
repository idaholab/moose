#ifndef COUPLEDCONVECTION_H_
#define COUPLEDCONVECTION_H_

#include "Kernel.h"

class CoupledConvection;

template<>
InputParameters validParams<CoupledConvection>();

/**
 * Define the Kernel for a convection operator that looks like:
 *
 * grad_some_var dot u'
 * 
 */
class CoupledConvection : public Kernel
{
public:
  CoupledConvection(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  VariableGradient & _velocity_vector;
};

#endif //COUPLEDCONVECTION_H
