//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PENALTYINCLINEDBC_H
#define PENALTYINCLINEDBC_H

#include "IntegratedBC.h"

class PenaltyInclinedBC;
class Function;

template <>
InputParameters validParams<PenaltyInclinedBC>();

/**
 * Weakly enforce an inclined BC (u\dot n = 0) using a penalty method.
 */
class PenaltyInclinedBC : public IntegratedBC
{
public:
  PenaltyInclinedBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const unsigned int _component;

  /// Coupled displacement variables
  unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<unsigned int> _disp_var;

private:
  Real _p;
};

#endif
