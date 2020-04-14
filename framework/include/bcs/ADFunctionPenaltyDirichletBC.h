//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class Function;

/**
 * A different approach to applying Dirichlet BCs
 *
 * uses \f$ \int(p u \cdot \phi)=\int(p f \cdot \phi)\f$ on \f$d\omega\f$
 *
 */
class ADFunctionPenaltyDirichletBC : public ADIntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  ADFunctionPenaltyDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Function that defines value of u on the boundary
  const Function & _func;

private:
  /// Penalty value
  const Real _p;
};
