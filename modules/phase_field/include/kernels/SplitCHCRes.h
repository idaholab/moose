/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHCRES_H
#define SPLITCHCRES_H

#include "SplitCHBase.h"

// Forward Declarations
class SplitCHCRes;

template <>
InputParameters validParams<SplitCHCRes>();

/// The couple, SplitCHCRes and SplitCHWRes, splits the CH equation by replacing chemical potential with 'w'.
class SplitCHCRes : public SplitCHBase
{
public:
  SplitCHCRes(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _kappa;
  unsigned int _w_var;
  const VariableValue & _w;
};

#endif // SPLITCHCRES_H
