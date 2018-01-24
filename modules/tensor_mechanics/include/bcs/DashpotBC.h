//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DASHPOTBC_H
#define DASHPOTBC_H

#include "IntegratedBC.h"

// Forward Declarations
class DashpotBC;

template <>
InputParameters validParams<DashpotBC>();

/**
 * Implements a simple constant Dashpot BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class DashpotBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DashpotBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _component;
  Real _coefficient;

  unsigned int _disp_x_var;
  unsigned int _disp_y_var;
  unsigned int _disp_z_var;

  const VariableValue & _disp_x_dot;
  const VariableValue & _disp_y_dot;
  const VariableValue & _disp_z_dot;
};

#endif // DASHPOTBC_H
