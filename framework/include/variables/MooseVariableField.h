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
    _curl_phi = &_assembly.feCurlPhi<OutputType>(_fe_type);
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
    _curl_phi_face = &_assembly.feCurlPhiFace<OutputType>(_fe_type);
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
    _curl_phi_neighbor = &_assembly.feCurlPhiNeighbor<OutputType>(_fe_type);
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
    _curl_phi_face_neighbor = &_assembly.feCurlPhiFaceNeighbor<OutputType>(_fe_type);
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
  const FieldVariableGradient & gradSlnDot()
  {
    _need_grad_dot = true;
    return _grad_u_dot;
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
  const VariableValue & duDotDu() { return _du_dot_du; }

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
  const FieldVariableGradient & gradSlnNeighborDot()
  {
    _need_grad_neighbor_dot = true;
    return _grad_u_neighbor_dot;
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
  const VariableValue & duDotDuNeighbor() { return _du_dot_du_neighbor; }

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

  /**
   * Return phi size
   */
  virtual size_t phiSize() override { return _phi.size(); }
  /**
   * Return phiFace size
   */
  virtual size_t phiFaceSize() override { return _phi_face.size(); }
  /**
   * Return phiNeighbor size
   */
  virtual size_t phiNeighborSize() override { return _phi_neighbor.size(); }
  /**
   * Return phiFaceNeighbor size
   */
  virtual size_t phiFaceNeighborSize() override { return _phi_face_neighbor.size(); }

  const OutputType & nodalValue();
  const OutputType & nodalValueOld();
  const OutputType & nodalValueOlder();
  const OutputType & nodalValuePreviousNL();
  const OutputType & nodalValueDot();
  const OutputType & nodalValueDuDotDu();
  const OutputType & nodalValueNeighbor();
  const OutputType & nodalValueOldNeighbor();
  const OutputType & nodalValueOlderNeighbor();
  const OutputType & nodalValuePreviousNLNeighbor();
  const OutputType & nodalValueDotNeighbor();
  const OutputType & nodalValueDuDotDuNeighbor();

  virtual void computeNodalValues() override;
  virtual void computeNodalNeighborValues() override;

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
  FieldVariableGradient _grad_u_dot;
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
  FieldVariableGradient _grad_u_neighbor_dot;
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
  VariableValue _du_dot_du, _du_dot_du_bak;
  VariableValue _du_dot_du_neighbor, _du_dot_du_bak_neighbor;

  /// Continuity type of the variable
  FEContinuity _continuity;

  /// Increment in the variable used in dampers
  FieldVariableValue _increment;

  /// Nodal values
  OutputType _nodal_value;
  OutputType _nodal_value_old;
  OutputType _nodal_value_older;
  OutputType _nodal_value_previous_nl;

  /// nodal values of u_dot
  OutputType _nodal_value_dot;

  friend class NodeFaceConstraint;
  friend class ValueThresholdMarker;
  friend class ValueRangeMarker;
};

#endif /* MOOSEVARIABLEFIELD_H */
