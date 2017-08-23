/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INERTIALFORCEEXP_H
#define INERTIALFORCEEXP_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class InertialForceExp;

template <>
InputParameters validParams<InertialForceExp>();

class InertialForceExp : public Kernel
{
public:
  InertialForceExp(const InputParameters & parameters);

  virtual void computeJacobian() override;

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

private:
  const MaterialProperty<Real> & _density;
  bool _lumped;
  const VariableValue & _u_old;
  const VariableValue & _u_older;
  const VariableValue & _u_nodal;
  const VariableValue & _u_nodal_old;
  const VariableValue & _u_nodal_older;
};

#endif // INERTIALFORCEEXP_H
