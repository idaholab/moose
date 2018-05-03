//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALDISPLACEMENTDIFFERENCEL2NORMPD_H
#define NODALDISPLACEMENTDIFFERENCEL2NORMPD_H

#include "NodalIntegralPostprocessorBasePD.h"

class NodalDisplacementDifferenceL2NormPD;

template <>
InputParameters validParams<NodalDisplacementDifferenceL2NormPD>();

/**
 * Postprocessor class to compute L2 norm of displacements difference between prediction and
 * analytical solution for peridynamic model
 */
class NodalDisplacementDifferenceL2NormPD : public NodalIntegralPostprocessorBasePD
{
public:
  NodalDisplacementDifferenceL2NormPD(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeNodalValue() override;

  ///@{ Known displacements analytical solutions
  const bool _has_func_0;
  Function * _func_0;

  const bool _has_func_1;
  Function * _func_1;

  const bool _has_func_2;
  Function * _func_2;
  ///@}

  /// Displacement variables
  std::vector<MooseVariableFEBase *> _disp_var;
};

#endif // NODALDISPLACEMENTDIFFERENCEL2NORMPD
