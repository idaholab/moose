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
#include "Assembly.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "ZeroInterface.h"
// libMesh
#include "libmesh/fe.h"
#include "libmesh/quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;
class ElementIndicator;

template<>
InputParameters validParams<ElementIndicator>();

class ElementIndicator :
  public Indicator,
  public TransientInterface,
  public PostprocessorInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public MaterialPropertyInterface,
  public ZeroInterface
{
public:
    ElementIndicator(const std::string & name, InputParameters parameters);
    virtual ~ElementIndicator(){};

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
    VariableValue & _u;
    /// Holds the solution gradient at the current quadrature points
    VariableGradient & _grad_u;

    /// Time derivative of u
    VariableValue & _u_dot;
    /// Derivative of u_dot wrt u
    VariableValue & _du_dot_du;

    /// Holds local indicator entries as their accumulated by this ElementIndicator
    DenseVector<Number> _local_indtr;
    //  Real _local_indtr;
};

#endif /* ELEMENTINDICATOR_H */
