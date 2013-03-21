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

#ifndef MOOSEVARIABLEBASE_H
#define MOOSEVARIABLEBASE_H

#include "MooseTypes.h"
#include "MooseArray.h"

#include "libmesh/variable.h"
#include "libmesh/dof_map.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

typedef MooseArray<Real>               VariableValue;
typedef MooseArray<RealGradient>       VariableGradient;
typedef MooseArray<RealTensor>         VariableSecond;

typedef MooseArray<std::vector<Real> >         VariableTestValue;
typedef MooseArray<std::vector<RealGradient> > VariableTestGradient;
typedef MooseArray<std::vector<RealTensor> >   VariableTestSecond;

typedef MooseArray<std::vector<Real> >         VariablePhiValue;
typedef MooseArray<std::vector<RealGradient> > VariablePhiGradient;
typedef MooseArray<std::vector<RealTensor> >   VariablePhiSecond;

class Assembly;
class SubProblem;
class SystemBase;


class MooseVariableBase
{
public:
  MooseVariableBase(unsigned int var_num, unsigned int index, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind);
  virtual ~MooseVariableBase();

  /**
   * Get the variable index.
   *
   * Used to index into the vector of residuals, jacobians blocks, etc.
   * @return The variable index
   */
  unsigned int index() const;

  /**
   * Get variable number coming from libMesh
   * @return the libmesh variable number
   */
  unsigned int number() const;

  /**
   * Get the system this variable is part of.
   */
  SystemBase & sys() { return _sys; }

  /**
   * Get the variable number
   */
  const std::string & name() const;

  /**
   * Kind of the variable (Nonlinear, Auxiliary, ...)
   */
  Moose::VarKindType kind() const;

  /**
   * Set the scaling factor for this variable
   */
  void scalingFactor(Real factor);

  /**
   * Get the scaling factor for this variable
   */
  Real scalingFactor() const;

  /**
   * Get the order of this variable
   */
  unsigned int order() const;

  /**
   * The DofMap associated with the system this variable is in.
   */
  const DofMap & dofMap() { return _dof_map; }

  std::vector<unsigned int> & dofIndices() { return _dof_indices; }

  /**
   * Is this variable nodal
   * @return true if it nodal, otherwise false
   */
  virtual bool isNodal() const;

protected:
  /// variable number (from libMesh)
  unsigned int _var_num;
  /// variable number within MOOSE
  unsigned int _index;
  Moose::VarKindType _var_kind;
  /// Problem this variable is part of
  SubProblem & _subproblem;
  /// System this variable is part of
  SystemBase & _sys;

  /// libMesh variable object for this variable
  const Variable & _variable;

  /// Assembly data
  Assembly & _assembly;
  /// DOF map
  const DofMap & _dof_map;
  /// DOF indices
  std::vector<unsigned int> _dof_indices;

  /// scaling factor for this variable
  Real _scaling_factor;

  /// true if this variable is non-linear
  bool _is_nl;
};

#endif /* MOOSEVARIABLEBASE_H */
