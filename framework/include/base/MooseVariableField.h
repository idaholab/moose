//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEVARIABLEFIELD_H
#define MOOSEVARIABLEFIELD_H

#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseMesh.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_vector.h"
#include "libmesh/tensor_tools.h"

/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 */
template <typename OutputType>
class MooseVariableField : public MooseVariableFE
{
  typedef OutputType OutputShape;
  typedef OutputType OutputValue;
  typedef typename TensorTools::IncrementRank<OutputShape>::type OutputGradient;
  typedef typename TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename TensorTools::DecrementRank<OutputShape>::type OutputDivergence;

  typedef MooseArray<OutputShape> FieldVariableValue;
  typedef MooseArray<OutputGradient> FieldVariableGradient;
  typedef MooseArray<OutputSecond> FieldVariableSecond;
  typedef MooseArray<OutputShape> FieldVariableCurl;
  typedef MooseArray<OutputDivergence> FieldVariableDivergence;

  typedef MooseArray<std::vector<OutputShape>> FieldVariablePhiValue;
  typedef MooseArray<std::vector<OutputGradient>> FieldVariablePhiGradient;
  typedef MooseArray<std::vector<OutputSecond>> FieldVariablePhiSecond;
  typedef MooseArray<std::vector<OutputShape>> FieldVariablePhiCurl;
  typedef MooseArray<std::vector<OutputDivergence>> FieldVariablePhiDivergence;

  typedef MooseArray<std::vector<OutputShape>> FieldVariableTestValue;
  typedef MooseArray<std::vector<OutputGradient>> FieldVariableTestGradient;
  typedef MooseArray<std::vector<OutputSecond>> FieldVariableTestSecond;
  typedef MooseArray<std::vector<OutputShape>> FieldVariableTestCurl;
  typedef MooseArray<std::vector<OutputDivergence>> FieldVariableTestDivergence;

public:
  MooseVariableField(unsigned int var_num,
                     const FEType & fe_type,
                     SystemBase & sys,
                     Assembly & assembly,
                     Moose::VarKindType var_kind);
  virtual ~MooseVariableField();

  void prepareIC();

  const FieldVariablePhiValue & phi() { return _phi; }
  const FieldVariablePhiGradient & gradPhi() { return _grad_phi; }
  const FieldVariablePhiSecond & secondPhi()
  {
    _second_phi = &_assembly.feSecondPhi<OutputType>(_fe_type);
    return *_second_phi;
  }
  const FieldVariablePhiCurl & curlPhi()
  {
    _curl_phi = &_assembly.feCurlPhi(_fe_type);
    return *_curl_phi;
  }

  const FieldVariablePhiValue & phiFace() { return _phi_face; }
  const FieldVariablePhiGradient & gradPhiFace() { return _grad_phi_face; }
  const FieldVariablePhiSecond & secondPhiFace()
  {
    _second_phi_face = &_assembly.feSecondPhiFace<OutputType>(_fe_type);
    return *_second_phi_face;
  }
  const FieldVariablePhiCurl & curlPhiFace()
  {
    _curl_phi_face = &_assembly.feCurlPhiFace(_fe_type);
    return *_curl_phi_face;
  }

  const FieldVariablePhiValue & phiNeighbor() { return _phi_neighbor; }
  const FieldVariablePhiGradient & gradPhiNeighbor() { return _grad_phi_neighbor; }
  const FieldVariablePhiSecond & secondPhiNeighbor()
  {
    _second_phi_neighbor = &_assembly.feSecondPhiNeighbor<OutputType>(_fe_type);
    return *_second_phi_neighbor;
  }
  const FieldVariablePhiCurl & curlPhiNeighbor()
  {
    _curl_phi_neighbor = &_assembly.feCurlPhiNeighbor(_fe_type);
    return *_curl_phi_neighbor;
  }

  const FieldVariablePhiValue & phiFaceNeighbor() { return _phi_face_neighbor; }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() { return _grad_phi_face_neighbor; }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor()
  {
    _second_phi_face_neighbor = &_assembly.feSecondPhiFaceNeighbor<OutputType>(_fe_type);
    return *_second_phi_face_neighbor;
  }
  const FieldVariablePhiCurl & curlPhiFaceNeighbor()
  {
    _curl_phi_face_neighbor = &_assembly.feCurlPhiFaceNeighbor(_fe_type);
    return *_curl_phi_face_neighbor;
  }

  // damping
  FieldVariableValue & increment() { return _increment; }

  const FieldVariableValue & sln() { return _u; }
  const FieldVariableValue & slnOld()
  {
    _need_u_old = true;
    return _u_old;
  }
  const FieldVariableValue & slnOlder()
  {
    _need_u_older = true;
    return _u_older;
  }
  const FieldVariableValue & slnPreviousNL()
  {
    _need_u_previous_nl = true;
    return _u_previous_nl;
  }
  const FieldVariableGradient & gradSln() { return _grad_u; }
  const FieldVariableGradient & gradSlnOld()
  {
    _need_grad_old = true;
    return _grad_u_old;
  }
  const FieldVariableGradient & gradSlnOlder()
  {
    _need_grad_older = true;
    return _grad_u_older;
  }
  const FieldVariableGradient & gradSlnPreviousNL()
  {
    _need_grad_previous_nl = true;
    return _grad_u_previous_nl;
  }
  const FieldVariableSecond & secondSln()
  {
    _need_second = true;
    secondPhi();
    secondPhiFace();
    return _second_u;
  }
  const FieldVariableSecond & secondSlnOld()
  {
    _need_second_old = true;
    secondPhi();
    secondPhiFace();
    return _second_u_old;
  }
  const FieldVariableSecond & secondSlnOlder()
  {
    _need_second_older = true;
    secondPhi();
    secondPhiFace();
    return _second_u_older;
  }
  const FieldVariableSecond & secondSlnPreviousNL()
  {
    _need_second_previous_nl = true;
    secondPhi();
    secondPhiFace();
    return _second_u_previous_nl;
  }
  const FieldVariableValue & curlSln()
  {
    _need_curl = true;
    return _curl_u;
  }
  const FieldVariableValue & curlSlnOld()
  {
    _need_curl_old = true;
    return _curl_u_old;
  }
  const FieldVariableValue & curlSlnOlder()
  {
    _need_curl_older = true;
    return _curl_u_older;
  }

  const FieldVariableValue & uDot() { return _u_dot; }
  const FieldVariableValue & duDotDu() { return _du_dot_du; }

  const FieldVariableValue & slnNeighbor() { return _u_neighbor; }
  const FieldVariableValue & slnOldNeighbor()
  {
    _need_u_old_neighbor = true;
    return _u_old_neighbor;
  }
  const FieldVariableValue & slnOlderNeighbor()
  {
    _need_u_older_neighbor = true;
    return _u_older_neighbor;
  }
  const FieldVariableValue & slnPreviousNLNeighbor()
  {
    _need_u_previous_nl_neighbor = true;
    return _u_previous_nl_neighbor;
  }
  const FieldVariableGradient & gradSlnNeighbor() { return _grad_u_neighbor; }
  const FieldVariableGradient & gradSlnOldNeighbor()
  {
    _need_grad_old_neighbor = true;
    return _grad_u_old_neighbor;
  }
  const FieldVariableGradient & gradSlnOlderNeighbor()
  {
    _need_grad_older_neighbor = true;
    return _grad_u_older_neighbor;
  }
  const FieldVariableGradient & gradSlnPreviousNLNeighbor()
  {
    _need_grad_previous_nl_neighbor = true;
    return _grad_u_previous_nl_neighbor;
  }
  const FieldVariableSecond & secondSlnNeighbor()
  {
    _need_second_neighbor = true;
    secondPhiFaceNeighbor();
    return _second_u_neighbor;
  }
  const FieldVariableSecond & secondSlnOldNeighbor()
  {
    _need_second_old_neighbor = true;
    secondPhiFaceNeighbor();
    return _second_u_old_neighbor;
  }
  const FieldVariableSecond & secondSlnOlderNeighbor()
  {
    _need_second_older_neighbor = true;
    secondPhiFaceNeighbor();
    return _second_u_older_neighbor;
  }
  const FieldVariableSecond & secondSlnPreviousNLNeighbor()
  {
    _need_second_previous_nl_neighbor = true;
    secondPhiFaceNeighbor();
    return _second_u_previous_nl_neighbor;
  }

  const FieldVariableCurl & curlSlnNeighbor()
  {
    _need_curl_neighbor = true;
    curlPhiFaceNeighbor();
    return _curl_u_neighbor;
  }
  const FieldVariableCurl & curlSlnOldNeighbor()
  {
    _need_curl_old_neighbor = true;
    curlPhiFaceNeighbor();
    return _curl_u_old_neighbor;
  }
  const FieldVariableCurl & curlSlnOlderNeighbor()
  {
    _need_curl_older_neighbor = true;
    curlPhiFaceNeighbor();
    return _curl_u_older_neighbor;
  }

  const FieldVariableValue & uDotNeighbor() { return _u_dot_neighbor; }
  const FieldVariableValue & duDotDuNeighbor() { return _du_dot_du_neighbor; }

  /**
   * Helper function for computing values
   */
  virtual void computeValuesHelper(QBase *& qrule,
                                   const FieldVariablePhiValue & phi,
                                   const FieldVariablePhiGradient & grad_phi,
                                   const FieldVariablePhiSecond *& second_phi,
                                   const FieldVariablePhiCurl *& curl_phi);

  /**
   * Helper function for computing values
   */
  virtual void computeNeighborValuesHelper(QBase *& qrule,
                                           const FieldVariablePhiValue & phi,
                                           const FieldVariablePhiGradient & grad_phi,
                                           const FieldVariablePhiSecond *& second_phi);

  /**
   * Compute values at interior quadrature points
   */
  virtual void computeElemValues() override;
  /**
   * Compute values at facial quadrature points
   */
  virtual void computeElemValuesFace() override;
  /**
   * Compute values at facial quadrature points for the neighbor
   */
  virtual void computeNeighborValuesFace() override;
  /**
   * Compute values at quadrature points for the neighbor
   */
  virtual void computeNeighborValues() override;
  /**
   * Set the nodal value for this variable to keep everything up to date
   */
  virtual void setNodalValue(Number value, unsigned int idx = 0) override;
  /**
   * Set values for this variable to keep everything up to date
   */
  virtual void setNodalValue(const DenseVector<Number> & value) override;

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  virtual void computeIncrementAtQps(const NumericVector<Number> & increment_vec);

  /**
   * Compute and store incremental change at the current node based on increment_vec
   */
  virtual void computeIncrementAtNode(const NumericVector<Number> & increment_vec);

  /**
   * Compute the variable value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable value
   */
  OutputType getValue(const Elem * elem, const std::vector<std::vector<OutputType>> & phi) const;

  /**
   * Compute the variable gradient value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable gradient value
   */
  typename OutputTools<OutputType>::OutputGradient
  getGradient(const Elem * elem,
              const std::vector<std::vector<typename OutputTools<OutputType>::OutputGradient>> &
                  grad_phi) const;

protected:
  // Shape function values, gradients, second derivatives
  const FieldVariablePhiValue & _phi;
  const FieldVariablePhiGradient & _grad_phi;
  const FieldVariablePhiSecond * _second_phi;
  const FieldVariablePhiCurl * _curl_phi;

  // Values, gradients and second derivatives of shape function on faces
  const FieldVariablePhiValue & _phi_face;
  const FieldVariablePhiGradient & _grad_phi_face;
  const FieldVariablePhiSecond * _second_phi_face;
  const FieldVariablePhiCurl * _curl_phi_face;

  // Values, gradients and second derivatives of shape function
  const FieldVariablePhiValue & _phi_neighbor;
  const FieldVariablePhiGradient & _grad_phi_neighbor;
  const FieldVariablePhiSecond * _second_phi_neighbor;
  const FieldVariablePhiCurl * _curl_phi_neighbor;

  // Values, gradients and second derivatives of shape function on faces
  const FieldVariablePhiValue & _phi_face_neighbor;
  const FieldVariablePhiGradient & _grad_phi_face_neighbor;
  const FieldVariablePhiSecond * _second_phi_face_neighbor;
  const FieldVariablePhiCurl * _curl_phi_face_neighbor;

  FieldVariableValue _u, _u_bak;
  FieldVariableValue _u_old, _u_old_bak;
  FieldVariableValue _u_older, _u_older_bak;
  FieldVariableValue _u_previous_nl;
  FieldVariableGradient _grad_u, _grad_u_bak;
  FieldVariableGradient _grad_u_old, _grad_u_old_bak;
  FieldVariableGradient _grad_u_older, _grad_u_older_bak;
  FieldVariableGradient _grad_u_previous_nl;
  FieldVariableSecond _second_u, _second_u_bak;
  FieldVariableSecond _second_u_old, _second_u_old_bak;
  FieldVariableSecond _second_u_older, _second_u_older_bak;
  FieldVariableSecond _second_u_previous_nl;
  FieldVariableCurl _curl_u, _curl_u_bak;
  FieldVariableCurl _curl_u_old, _curl_u_old_bak;
  FieldVariableCurl _curl_u_older;

  FieldVariableValue _u_neighbor;
  FieldVariableValue _u_old_neighbor;
  FieldVariableValue _u_older_neighbor;
  FieldVariableValue _u_previous_nl_neighbor;
  FieldVariableGradient _grad_u_neighbor;
  FieldVariableGradient _grad_u_old_neighbor;
  FieldVariableGradient _grad_u_older_neighbor;
  FieldVariableGradient _grad_u_previous_nl_neighbor;
  FieldVariableSecond _second_u_neighbor;
  FieldVariableSecond _second_u_old_neighbor;
  FieldVariableSecond _second_u_older_neighbor;
  FieldVariableSecond _second_u_previous_nl_neighbor;
  FieldVariableCurl _curl_u_neighbor;
  FieldVariableCurl _curl_u_old_neighbor;
  FieldVariableCurl _curl_u_older_neighbor;

  // time derivatives

  /// u_dot (time derivative)
  FieldVariableValue _u_dot, _u_dot_bak;
  FieldVariableValue _u_dot_neighbor, _u_dot_bak_neighbor;

  /// derivative of u_dot wrt u
  FieldVariableValue _du_dot_du, _du_dot_du_bak;
  FieldVariableValue _du_dot_du_neighbor, _du_dot_du_bak_neighbor;

  /// Continuity type of the variable
  FEContinuity _continuity;

  /// Increment in the variable used in dampers
  FieldVariableValue _increment;

  friend class NodeFaceConstraint;
  friend class ValueThresholdMarker;
  friend class ValueRangeMarker;
};

template <typename OutputType>
MooseVariableField<OutputType>::MooseVariableField(unsigned int var_num,
                                                   const FEType & fe_type,
                                                   SystemBase & sys,
                                                   Assembly & assembly,
                                                   Moose::VarKindType var_kind)
  : MooseVariableFE(var_num, fe_type, sys, var_kind, assembly),
    _phi(_assembly.fePhi<OutputType>(_fe_type)),
    _grad_phi(_assembly.feGradPhi<OutputType>(_fe_type)),
    _phi_face(_assembly.fePhiFace<OutputType>(_fe_type)),
    _grad_phi_face(_assembly.feGradPhiFace<OutputType>(_fe_type)),
    _phi_neighbor(_assembly.fePhiNeighbor<OutputType>(_fe_type)),
    _grad_phi_neighbor(_assembly.feGradPhiNeighbor<OutputType>(_fe_type)),
    _phi_face_neighbor(_assembly.fePhiFaceNeighbor<OutputType>(_fe_type)),
    _grad_phi_face_neighbor(_assembly.feGradPhiFaceNeighbor<OutputType>(_fe_type))
{
  // FIXME: continuity of FE type seems equivalent with the definition of nodal variables.
  //        Continuity does not depend on the FE dimension, so we just pass in a valid dimension.
  if (_fe_type.family == NEDELEC_ONE || _fe_type.family == LAGRANGE_VEC)
    _continuity = _assembly.getVectorFE(_fe_type, _sys.mesh().dimension())->get_continuity();
  else
    _continuity = _assembly.getFE(_fe_type, _sys.mesh().dimension())->get_continuity();

  _is_nodal = (_continuity == C_ZERO || _continuity == C_ONE);
}

template <typename OutputType>
MooseVariableField<OutputType>::~MooseVariableField()
{
  _u.release();
  _u_bak.release();
  _u_old.release();
  _u_old_bak.release();
  _u_older.release();
  _u_older_bak.release();
  _u_previous_nl.release();

  _grad_u.release();
  _grad_u_bak.release();
  _grad_u_old.release();
  _grad_u_old_bak.release();
  _grad_u_older.release();
  _grad_u_older.release();
  _grad_u_previous_nl.release();

  _second_u.release();
  _second_u_bak.release();
  _second_u_old.release();
  _second_u_old_bak.release();
  _second_u_older.release();
  _second_u_older_bak.release();
  _second_u_previous_nl.release();

  _curl_u.release();
  _curl_u_bak.release();
  _curl_u_old.release();
  _curl_u_old_bak.release();
  _curl_u_older.release();

  _u_dot.release();
  _u_dot_bak.release();
  _u_dot_neighbor.release();
  _u_dot_bak_neighbor.release();

  _du_dot_du.release();
  _du_dot_du_bak.release();
  _du_dot_du_neighbor.release();
  _du_dot_du_bak_neighbor.release();

  _increment.release();

  _u_neighbor.release();
  _u_old_neighbor.release();
  _u_older_neighbor.release();
  _u_previous_nl_neighbor.release();

  _grad_u_neighbor.release();
  _grad_u_old_neighbor.release();
  _grad_u_older_neighbor.release();
  _grad_u_previous_nl_neighbor.release();

  _second_u_neighbor.release();
  _second_u_old_neighbor.release();
  _second_u_older_neighbor.release();
  _second_u_previous_nl_neighbor.release();

  _curl_u_neighbor.release();
  _curl_u_old_neighbor.release();
  _curl_u_older_neighbor.release();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::prepareIC()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _dof_u.resize(_dof_indices.size());

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeElemValues()
{
  computeValuesHelper(_qrule, _phi, _grad_phi, _second_phi, _curl_phi);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeElemValuesFace()
{
  computeValuesHelper(_qrule_face, _phi_face, _grad_phi_face, _second_phi_face, _curl_phi_face);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeNeighborValuesFace()
{
  computeNeighborValuesHelper(
      _qrule_neighbor, _phi_face_neighbor, _grad_phi_face_neighbor, _second_phi_face_neighbor);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeNeighborValues()
{
  computeNeighborValuesHelper(
      _qrule_neighbor, _phi_neighbor, _grad_phi_neighbor, _second_phi_neighbor);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::setNodalValue(const DenseVector<Number> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _dof_u[i] = values(i);

  _has_nodal_value = true;

  for (unsigned int qp = 0; qp < _u.size(); qp++)
  {
    _u[qp] = 0;
    for (unsigned int i = 0; i < _dof_u.size(); i++)
      _u[qp] += _phi[i][qp] * _dof_u[i];
  }
}

template <typename OutputType>
void
MooseVariableField<OutputType>::setNodalValue(Number value, unsigned int idx /* = 0*/)
{
  _dof_u[idx] = value; // update variable nodal value
  _has_nodal_value = true;

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeIncrementAtQps(const NumericVector<Number> & increment_vec)
{
  unsigned int nqp = _qrule->n_points();

  _increment.resize(nqp);
  // Compute the increment at each quadrature point
  unsigned int num_dofs = _dof_indices.size();
  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    _increment[qp] = 0;
    for (unsigned int i = 0; i < num_dofs; i++)
      _increment[qp] += _phi[i][qp] * increment_vec(_dof_indices[i]);
  }
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeIncrementAtNode(const NumericVector<Number> & increment_vec)
{
  if (!isNodal())
    mooseError("computeIncrementAtNode can only be called for nodal variables");

  _increment.resize(1);

  // Compute the increment for the current DOF
  _increment[0] = increment_vec(_dof_indices[0]);
}

template <typename OutputType>
OutputType
MooseVariableField<OutputType>::getValue(const Elem * elem,
                                         const std::vector<std::vector<OutputType>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  OutputType value = 0;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = (*_sys.currentSolution())(dof_indices[0]);
  }

  return value;
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeValuesHelper(QBase *& qrule,
                                                    const FieldVariablePhiValue & phi,
                                                    const FieldVariablePhiGradient & grad_phi,
                                                    const FieldVariablePhiSecond *& second_phi,
                                                    const FieldVariablePhiValue *& curl_phi)

{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    _u_dot.resize(nqp);
    _du_dot_du.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (_need_curl)
      _curl_u[i] = 0;

    if (_need_u_previous_nl)
      _u_previous_nl[i] = 0;

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[i] = 0;

    if (_need_second_previous_nl)
      _second_u_previous_nl[i] = 0;

    if (is_transient)
    {
      _u_dot[i] = 0;
      _du_dot_du[i] = 0;

      if (_need_u_old)
        _u_old[i] = 0;

      if (_need_u_older)
        _u_older[i] = 0;

      if (_need_grad_old)
        _grad_u_old[i] = 0;

      if (_need_grad_older)
        _grad_u_older[i] = 0;

      if (_need_second_old)
        _second_u_old[i] = 0;

      if (_need_second_older)
        _second_u_older[i] = 0;

      if (_need_curl_old)
        _curl_u_old[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices.size();

  if (_need_dof_u)
    _dof_u.resize(num_dofs);

  if (_need_dof_u_previous_nl)
    _dof_u_previous_nl.resize(num_dofs);

  if (is_transient)
  {
    if (_need_dof_u_old)
      _dof_u_old.resize(num_dofs);
    if (_need_dof_u_older)
      _dof_u_older.resize(num_dofs);
    if (_need_dof_u_dot)
      _dof_u_dot.resize(num_dofs);
  }

  if (_need_solution_dofs)
    _solution_dofs.resize(num_dofs);

  if (_need_solution_dofs_old)
    _solution_dofs_old.resize(num_dofs);

  if (_need_solution_dofs_older)
    _solution_dofs_older.resize(num_dofs);

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> * solution_prev_nl = _sys.solutionPreviousNewton();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx = 0;
  Real soln_local = 0;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real soln_previous_nl_local = 0;
  Real u_dot_local = 0;

  const OutputType * phi_local = NULL;
  const typename OutputTools<OutputType>::OutputGradient * dphi_qp = NULL;
  const typename OutputTools<OutputType>::OutputSecond * d2phi_local = NULL;
  const OutputType * curl_phi_local = NULL;

  typename OutputTools<OutputType>::OutputGradient * grad_u_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_old_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_older_qp = NULL;
  typename OutputTools<OutputType>::OutputGradient * grad_u_previous_nl_qp = NULL;

  typename OutputTools<OutputType>::OutputSecond * second_u_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_old_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_older_qp = NULL;
  typename OutputTools<OutputType>::OutputSecond * second_u_previous_nl_qp = NULL;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    idx = _dof_indices[i];
    soln_local = current_solution(idx);

    if (_need_dof_u)
      _dof_u[i] = soln_local;

    if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
        _need_dof_u_previous_nl)
      soln_previous_nl_local = (*solution_prev_nl)(idx);

    if (_need_dof_u_previous_nl)
      _dof_u_previous_nl[i] = soln_previous_nl_local;

    if (_need_solution_dofs)
      _solution_dofs(i) = soln_local;

    if (is_transient)
    {
      if (_need_u_old || _need_grad_old || _need_second_old || _need_dof_u_old)
        soln_old_local = solution_old(idx);

      if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_u_older)
        soln_older_local = solution_older(idx);

      if (_need_dof_u_old)
        _dof_u_old[i] = soln_old_local;
      if (_need_dof_u_older)
        _dof_u_older[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_dof_u_dot)
        _dof_u_dot[i] = u_dot_local;

      if (_need_solution_dofs_old)
        _solution_dofs_old(i) = solution_old(idx);

      if (_need_solution_dofs_older)
        _solution_dofs_older(i) = solution_older(idx);
    }

    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      phi_local = &phi[i][qp];
      dphi_qp = &grad_phi[i][qp];

      grad_u_qp = &_grad_u[qp];

      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp = &_grad_u_previous_nl[qp];

      if (is_transient)
      {
        if (_need_grad_old)
          grad_u_old_qp = &_grad_u_old[qp];

        if (_need_grad_older)
          grad_u_older_qp = &_grad_u_older[qp];
      }

      if (_need_second || _need_second_old || _need_second_older || _need_second_previous_nl)
      {
        d2phi_local = &(*second_phi)[i][qp];

        if (_need_second)
        {
          second_u_qp = &_second_u[qp];
          second_u_qp->add_scaled(*d2phi_local, soln_local);
        }

        if (_need_second_previous_nl)
        {
          second_u_previous_nl_qp = &_second_u_previous_nl[qp];
          second_u_previous_nl_qp->add_scaled(*d2phi_local, soln_previous_nl_local);
        }

        if (is_transient)
        {
          if (_need_second_old)
            second_u_old_qp = &_second_u_old[qp];

          if (_need_second_older)
            second_u_older_qp = &_second_u_older[qp];
        }
      }

      if (_need_curl || _need_curl_old)
      {
        curl_phi_local = &(*curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += *curl_phi_local * soln_local;

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += *curl_phi_local * soln_old_local;
      }

      _u[qp] += *phi_local * soln_local;

      grad_u_qp->add_scaled(*dphi_qp, soln_local);

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += *phi_local * soln_previous_nl_local;
      if (_need_grad_previous_nl)
        grad_u_previous_nl_qp->add_scaled(*dphi_qp, soln_previous_nl_local);

      if (is_transient)
      {
        _u_dot[qp] += *phi_local * u_dot_local;
        _du_dot_du[qp] = du_dot_du;

        if (_need_u_old)
          _u_old[qp] += *phi_local * soln_old_local;

        if (_need_u_older)
          _u_older[qp] += *phi_local * soln_older_local;

        if (_need_grad_old)
          grad_u_old_qp->add_scaled(*dphi_qp, soln_old_local);

        if (_need_grad_older)
          grad_u_older_qp->add_scaled(*dphi_qp, soln_older_local);

        if (_need_second_old)
          second_u_old_qp->add_scaled(*d2phi_local, soln_old_local);

        if (_need_second_older)
          second_u_older_qp->add_scaled(*d2phi_local, soln_older_local);
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableField<OutputType>::computeNeighborValuesHelper(
    QBase *& qrule,
    const FieldVariablePhiValue & phi,
    const FieldVariablePhiGradient & grad_phi,
    const FieldVariablePhiSecond *& second_phi)
{
  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = qrule->n_points();

  _u_neighbor.resize(nqp);
  _grad_u_neighbor.resize(nqp);

  if (_need_second_neighbor)
    _second_u_neighbor.resize(nqp);

  if (is_transient)
  {
    _u_dot_neighbor.resize(nqp);
    _du_dot_du_neighbor.resize(nqp);

    if (_need_u_old_neighbor)
      _u_old_neighbor.resize(nqp);

    if (_need_u_older_neighbor)
      _u_older_neighbor.resize(nqp);

    if (_need_grad_old_neighbor)
      _grad_u_old_neighbor.resize(nqp);

    if (_need_grad_older_neighbor)
      _grad_u_older_neighbor.resize(nqp);

    if (_need_second_old_neighbor)
      _second_u_old_neighbor.resize(nqp);

    if (_need_second_older_neighbor)
      _second_u_older_neighbor.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u_neighbor[i] = 0;
    _grad_u_neighbor[i] = 0;

    if (_need_second_neighbor)
      _second_u_neighbor[i] = 0;

    if (is_transient)
    {
      _u_dot_neighbor[i] = 0;
      _du_dot_du_neighbor[i] = 0;

      if (_need_u_old_neighbor)
        _u_old_neighbor[i] = 0;

      if (_need_u_older_neighbor)
        _u_older_neighbor[i] = 0;

      if (_need_grad_old_neighbor)
        _grad_u_old_neighbor[i] = 0;

      if (_need_grad_older_neighbor)
        _grad_u_older_neighbor[i] = 0;

      if (_need_second_old_neighbor)
        _second_u_old_neighbor[i] = 0;

      if (_need_second_older_neighbor)
        _second_u_older_neighbor[i] = 0;
    }
  }

  unsigned int num_dofs = _dof_indices_neighbor.size();

  if (_need_dof_u_neighbor)
    _dof_u_neighbor.resize(num_dofs);
  if (is_transient)
  {
    if (_need_dof_u_old_neighbor)
      _dof_u_old_neighbor.resize(num_dofs);
    if (_need_dof_u_older_neighbor)
      _dof_u_older_neighbor.resize(num_dofs);
    if (_need_dof_u_dot_neighbor)
      _dof_u_dot_neighbor.resize(num_dofs);
  }

  if (_need_solution_dofs_neighbor)
    _solution_dofs_neighbor.resize(num_dofs);

  if (_need_solution_dofs_old_neighbor)
    _solution_dofs_old_neighbor.resize(num_dofs);

  if (_need_solution_dofs_older_neighbor)
    _solution_dofs_older_neighbor.resize(num_dofs);

  const NumericVector<Real> & current_solution = *_sys.currentSolution();
  const NumericVector<Real> & solution_old = _sys.solutionOld();
  const NumericVector<Real> & solution_older = _sys.solutionOlder();
  const NumericVector<Real> & u_dot = _sys.solutionUDot();
  const Real & du_dot_du = _sys.duDotDu();

  dof_id_type idx;
  Real soln_local;
  Real soln_old_local = 0;
  Real soln_older_local = 0;
  Real u_dot_local = 0;

  OutputType phi_local;
  typename OutputTools<OutputType>::OutputGradient dphi_local;
  typename OutputTools<OutputType>::OutputSecond d2phi_local;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    idx = _dof_indices_neighbor[i];
    soln_local = current_solution(idx);

    if (_need_dof_u_neighbor)
      _dof_u_neighbor[i] = soln_local;

    if (_need_solution_dofs_neighbor)
      _solution_dofs_neighbor(i) = soln_local;

    if (is_transient)
    {
      if (_need_u_old_neighbor)
        soln_old_local = solution_old(idx);

      if (_need_u_older_neighbor)
        soln_older_local = solution_older(idx);

      if (_need_dof_u_old_neighbor)
        _dof_u_old_neighbor[i] = soln_old_local;
      if (_need_dof_u_older_neighbor)
        _dof_u_older_neighbor[i] = soln_older_local;

      u_dot_local = u_dot(idx);
      if (_need_dof_u_dot_neighbor)
        _dof_u_dot_neighbor[i] = u_dot_local;

      if (_need_solution_dofs_old_neighbor)
        _solution_dofs_old_neighbor(i) = solution_old(idx);

      if (_need_solution_dofs_older_neighbor)
        _solution_dofs_older_neighbor(i) = solution_older(idx);
    }

    for (unsigned int qp = 0; qp < nqp; ++qp)
    {
      phi_local = phi[i][qp];
      dphi_local = grad_phi[i][qp];

      if (_need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor)
        d2phi_local = (*second_phi)[i][qp];

      _u_neighbor[qp] += phi_local * soln_local;
      _grad_u_neighbor[qp] += dphi_local * soln_local;

      if (_need_second_neighbor)
        _second_u_neighbor[qp] += d2phi_local * soln_local;

      if (is_transient)
      {
        _u_dot_neighbor[qp] += phi_local * u_dot_local;
        _du_dot_du_neighbor[qp] = du_dot_du;

        if (_need_u_old_neighbor)
          _u_old_neighbor[qp] += phi_local * soln_old_local;

        if (_need_u_older_neighbor)
          _u_older_neighbor[qp] += phi_local * soln_older_local;

        if (_need_grad_old_neighbor)
          _grad_u_old_neighbor[qp] += dphi_local * soln_old_local;

        if (_need_grad_older_neighbor)
          _grad_u_older_neighbor[qp] += dphi_local * soln_older_local;

        if (_need_second_old_neighbor)
          _second_u_old_neighbor[qp] += d2phi_local * soln_old_local;

        if (_need_second_older_neighbor)
          _second_u_older_neighbor[qp] += d2phi_local * soln_older_local;
      }
    }
  }
}

template <typename OutputType>
typename OutputTools<OutputType>::OutputGradient
MooseVariableField<OutputType>::getGradient(
    const Elem * elem,
    const std::vector<std::vector<typename OutputTools<OutputType>::OutputGradient>> & grad_phi)
    const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  typename OutputTools<OutputType>::OutputGradient value;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += grad_phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = 0.0;
  }

  return value;
}

#endif /* MOOSEVARIABLEFIELD_H */
