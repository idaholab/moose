/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTINDICATOR_H
#define ELEMENTINDICATOR_H

#include "Indicator.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "MooseVariableInterface.h"
#include "MaterialPropertyInterface.h"
#include "ZeroInterface.h"

// Forward declarations
class ElementIndicator;
class MooseVariable;

template<>
InputParameters validParams<ElementIndicator>();

class ElementIndicator :
  public Indicator,
  public TransientInterface,
  public PostprocessorInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableDependencyInterface,
  public MooseVariableInterface,
  public MaterialPropertyInterface,
  public ZeroInterface
{
public:
  ElementIndicator(const InputParameters & parameters);

protected:
  MooseVariable & _field_var;

  const Elem * & _current_elem;
  /// Volume of the current element
  const Real & _current_elem_volume;

  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  MooseVariable & _var;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Time derivative of u
  const VariableValue & _u_dot;

  /// Derivative of u_dot wrt u
  const VariableValue & _du_dot_du;

  /// Holds local indicator entries as their accumulated by this ElementIndicator
  DenseVector<Number> _local_indtr;
};

#endif /* ELEMENTINDICATOR_H */
