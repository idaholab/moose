//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Indicator.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableInterface.h"
#include "MaterialPropertyInterface.h"

template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;

class ElementIndicator : public Indicator,
                         public TransientInterface,
                         public PostprocessorInterface,
                         public Coupleable,
                         public ScalarCoupleable,
                         public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  ElementIndicator(const InputParameters & parameters);

protected:
  MooseVariable & _field_var;

  const Elem * const & _current_elem;
  /// Volume of the current element
  const Real & _current_elem_volume;

  unsigned int _qp;
  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  MooseVariableField<Real> & _var;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Holds local indicator entries as their accumulated by this ElementIndicator
  DenseVector<Number> _local_indtr;
};
