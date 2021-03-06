#pragma once

#include "InterfaceKernel.h"

class ParallelElectricFieldInterface : public VectorInterfaceKernel
{
public:
  static InputParameters validParams();

  ParallelElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  RealVectorValue _u_parallel;
  RealVectorValue _secondary_parallel;

  RealVectorValue _phi_u_parallel;
  RealVectorValue _phi_secondary_parallel;
};
