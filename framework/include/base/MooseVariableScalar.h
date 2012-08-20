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

#ifndef MOOSEVARIABLESCALAR_H
#define MOOSEVARIABLESCALAR_H

#include "MooseArray.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"

// libMesh
#include "dof_map.h"

class Assembly;
class SubProblem;
class SystemBase;


/**
 * Class for scalar variables (they are different)
 *
 */
class MooseVariableScalar
{
public:
  MooseVariableScalar(unsigned int var_num, unsigned int mvn, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind);
  virtual ~MooseVariableScalar();

  void reinit();

  /**
   * Get the variable number
   */
  unsigned int number() { return _moose_var_num; }

  /**
   * Get the variable number
   */
  const std::string & name();

  /**
   * Kind of the variable (Nonlinear, Auxiliary, ...)
   */
  Moose::VarKindType kind() { return _var_kind; }

  /**
   * Get the order of this variable
   */
  unsigned int order() const;

  //
  VariableValue & sln() { return _u; }
  VariableValue & slnOld() { return _u_old; }

  VariableValue & uDot() { return _u_dot; }
  VariableValue & duDotDu() { return _du_dot_du; }

  const std::vector<unsigned int> & dofIndices() { return _dof_indices; }

  /**
   * Set the scaling factor for this variable
   */
  void scalingFactor(Real factor) { _scaling_factor = factor; }
  /**
   * Get the scaling factor for this variable
   */
  Real scalingFactor() { return _scaling_factor; }

  /**
   * Set the nodal value for this variable (to keep everything up to date
   */
  void setValue(unsigned int i, Number value);

  void insert(NumericVector<Number> & soln);

protected:
  /// variable number (from libMesh)
  unsigned int _var_num;
  /// variable number within MOOSE
  unsigned int _moose_var_num;
  Moose::VarKindType _var_kind;
  /// Problem this variable is part of
  SubProblem & _subproblem;
  /// System this variable is part of
  SystemBase & _sys;

  /// Assembly data
  Assembly & _assembly;
  /// DOF map
  const DofMap & _dof_map;
  /// DOF indices
  std::vector<unsigned int> _dof_indices;

  bool _has_value;
  /// The value of scalar variable
  VariableValue _u;
  /// The old value of scalar variable
  VariableValue _u_old;

  VariableValue _u_dot;
  VariableValue _du_dot_du;

  /// scaling factor for this variable
  Real _scaling_factor;

  /// true if this variable is non-linear
  bool _is_nl;
};

#endif /* MOOSEVARIABLESCALAR_H */
