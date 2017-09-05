/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MANAGEDSOURCETEST_H
#define MANAGEDSOURCETEST_H

#include "XFEMMaterialManagerDiracKernel.h"

class ManagedSourceTest;
class XFEMMaterialManager;

template <>
InputParameters validParams<ManagedSourceTest>();

/**
 * Stateful point source test kernel for the XFEMMaterialManager
 */
class ManagedSourceTest : public XFEMMaterialManagerDiracKernel
{
public:
  ManagedSourceTest(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

protected:
  const MaterialProperty<Real> * _prop1;
  const MaterialProperty<Real> * _prop3;
};

#endif // MANAGEDSOURCETEST_H
