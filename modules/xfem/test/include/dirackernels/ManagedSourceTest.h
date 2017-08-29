/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MANAGEDSOURCETEST_H
#define MANAGEDSOURCETEST_H

#include "DiracKernel.h"

class ManagedSourceTest;
class XFEMMaterialManager;

template <>
InputParameters validParams<ManagedSourceTest>();

/**
 * Stateful point source test kernel for the XFEMMaterialManager
 */
class ManagedSourceTest : public DiracKernel
{
public:
  ManagedSourceTest(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

protected:
  const XFEMMaterialManager & _manager;
};

#endif // MANAGEDSOURCETEST_H
