/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ElementDiffusionReaction_H
#define ElementDiffusionReaction_H

#include "KernelBase.h"

#include "ElementInterface.h"

class ElementDiffusionReaction;

template <>
InputParameters validParams<ElementDiffusionReaction>();

class ElementDiffusionReaction : public KernelBase, public ElementInterface
{
public:
  ElementDiffusionReaction(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

protected:
  const VariableValue & _u_coef;
  Real _d;
  Real _r;
  VariableValue _src;
};

#endif /* KERNEL_H */
