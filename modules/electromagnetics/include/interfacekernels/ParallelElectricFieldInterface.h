#pragma once

#include "InterfaceKernel.h"

class ParallelElectricFieldInterface;

template <>
InputParameters
validParams<ParallelElectricFieldInterface>();

/**
 *
 */
class ParallelElectricFieldInterface : public VectorInterfaceKernel
{
public:
  ParallelElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  RealVectorValue _u_parallel;
  RealVectorValue _neighbor_parallel;

  RealVectorValue _phi_u_parallel;
  RealVectorValue _phi_neighbor_parallel;
};
