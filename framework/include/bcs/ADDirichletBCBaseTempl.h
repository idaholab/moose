//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBase.h"
#include "ADNodalBC.h"

/**
 * Base class for automatic differentiation Dirichlet BCs
 */
template <typename T>
class ADDirichletBCBaseTempl : public ADNodalBCTempl<T, ADDirichletBCBase>
{
public:
  ADDirichletBCBaseTempl(const InputParameters & parameters);

  /**
   * Method to preset the nodal value if applicable
   */
  virtual void computeValue(NumericVector<Number> & current_solution) override;

  static InputParameters validParams();

protected:
  virtual typename Moose::ADType<T>::type computeQpResidual() override;

  /**
   * Compute the value of the Dirichlet BC at the current quadrature point
   */
  virtual typename Moose::ADType<T>::type computeQpValue() = 0;

  using ADNodalBCTempl<T, ADDirichletBCBase>::_var;
  using ADNodalBCTempl<T, ADDirichletBCBase>::_sys;
  using ADNodalBCTempl<T, ADDirichletBCBase>::_current_node;
  using ADNodalBCTempl<T, ADDirichletBCBase>::_u;
  using ADNodalBCTempl<T, ADDirichletBCBase>::_t;
  using ADNodalBCTempl<T, ADDirichletBCBase>::shouldSetComp;
};
