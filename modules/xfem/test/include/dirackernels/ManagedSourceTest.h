/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#pragma once

#include "XFEMMaterialManagerDiracKernel.h"

class XFEMMaterialManager;

/**
 * Stateful point source test kernel for the XFEMMaterialManager
 */
class ManagedSourceTest : public XFEMMaterialManagerDiracKernel
{
public:
  static InputParameters validParams();

  ManagedSourceTest(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

protected:
  const MaterialProperty<Real> * _prop1;
  const MaterialProperty<Real> * _prop3;
};
