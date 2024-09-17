//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInitialConditionBase.h"

// libMesh
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/elem.h"

// forward declarations
class FEProblemBase;

/**
 * This is a template class that implements the workhorse `compute` and `computeNodal` methods. The
 * former method is used for setting block initial conditions. It first projects the initial
 * condition field to nodes, then to edges, then to faces, then to interior dofs. The latter
 * `computeNodal` method sets dof values for boundary restricted initial conditions
 */
template <typename T>
class FVInitialConditionTempl : public FVInitialConditionBase
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  FVInitialConditionTempl(const InputParameters & parameters);

  virtual ~FVInitialConditionTempl();

  static InputParameters validParams();

  virtual MooseVariableFEBase & variable() override { return _base_var; }

  virtual void computeElement(const ElemInfo & elem_info) override;

  /**
   * The value of the variable at a point.
   *
   * This must be overridden by derived classes.
   */
  virtual T value(const Point & p) = 0;

protected:
  FEProblemBase & _fe_problem;
  THREAD_ID _tid;

  /// Time
  Real & _t;

  /// The variable that this initial condition is acting upon.
  MooseVariableField<T> & _base_var;

  /// the mesh dimension
  unsigned int _dim;
};

template <typename T>
InputParameters
FVInitialConditionTempl<T>::validParams()
{
  return FVInitialConditionBase::validParams();
}

typedef FVInitialConditionTempl<Real> FVInitialCondition;

// Prevent implicit instantiation in other translation units where these classes are used
extern template class FVInitialConditionTempl<Real>;
