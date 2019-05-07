
#ifndef INTERFACEREACTION_H
#define INTERFACEREACTION_H

#include "InterfaceKernel.h"

// Forward Declarations
class InterfaceReaction;

template <>
InputParameters validParams<InterfaceReaction>();

class InterfaceReaction : public InterfaceKernel
{
public:
  InterfaceReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  Real _kf;
  Real _kb;
  const MaterialProperty<Real> & _D;
  const MaterialProperty<Real> & _D_neighbor;
};

#endif
