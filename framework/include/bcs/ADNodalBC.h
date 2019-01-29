//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADNODALBC_H
#define ADNODALBC_H

#include "NodalBCBase.h"
#include "MooseVariableInterface.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
template <typename T, ComputeStage compute_stage>
class ADNodalBCTempl : public NodalBCBase, public MooseVariableInterface<T>
{
public:
  ADNodalBCTempl(const InputParameters & parameters);

  virtual MooseVariableFE<T> & variable() override { return _var; }

  void computeResidual() override;
  void computeJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  /**
   * Compute this NodalBC's contribution to the residual at the current quadrature point
   */
  virtual typename Moose::ValueType<T, compute_stage>::type computeQpResidual() = 0;

  /// The variable that this NodalBC operates on
  MooseVariableFE<T> & _var;

  /// current node being processed
  const Node *& _current_node;

  /// Value of the unknown variable this BC is acting on
  const typename Moose::ValueType<T, compute_stage>::type & _u;
};

template <ComputeStage compute_stage>
using ADNodalBC = ADNodalBCTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorNodalBC = ADNodalBCTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADNodalBC);
declareADValidParams(ADVectorNodalBC);

#define usingTemplNodalBCMembers(type)                                                             \
  using ADNodalBCTempl<type, compute_stage>::_u;                                                   \
  using ADNodalBCTempl<type, compute_stage>::_var;                                                 \
  using ADNodalBCTempl<type, compute_stage>::_current_node;                                        \
  using ADNodalBCTempl<type, compute_stage>::_t;                                                   \
  using ADNodalBCTempl<type, compute_stage>::getFunction;                                          \
  using ADNodalBCTempl<type, compute_stage>::variable;                                             \
  using ADNodalBCTempl<type, compute_stage>::paramError;                                           \
  using ADNodalBCTempl<type, compute_stage>::isParamValid

#define usingNodalBCMembers usingTemplNodalBCMembers(Real)
#define usingVectorNodalBCMembers usingTemplNodalBCMembers(RealVectorValue)

#endif /* ADNODALBC_H */
