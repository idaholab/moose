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

#ifndef ARRAYMOOSEVARIABLE_H
#define ARRAYMOOSEVARIABLE_H

#include "MooseVariableBase.h"
#include "ParallelUniqueId.h"

#include "libmesh/dense_matrix.h"

#include "Eigen/Core"

// Forward declarations
class Assembly;
class SubProblem;
class SystemBase;

// libMesh forward declarations
namespace libMesh
{
class Elem;
class Node;
class QBase;
template <typename T> class NumericVector;
template <typename T> class DenseVector;
}

typedef MooseArray<Eigen::VectorXd>       ArrayVariableValue;
typedef MooseArray<Eigen::Matrix<Real, Eigen::Dynamic, LIBMESH_DIM> >       ArrayVariableGradient;
typedef MooseArray<RealTensor>         ArrayVariableSecond;

// typedef MooseArray<std::vector<Real> >         VariableTestValue;
typedef std::vector<std::vector<Eigen::Map<Eigen::Vector3d> > >       ArrayVariableTestGradient;
typedef MooseArray<std::vector<RealTensor> >   VariableTestSecond;
//
// typedef MooseArray<std::vector<Real> >         VariablePhiValue;
typedef std::vector<std::vector<Eigen::Map<Eigen::Vector3d> > > ArrayVariablePhiGradient;
// typedef MooseArray<std::vector<RealTensor> >   VariablePhiSecond;

class Assembly;
class SubProblem;
class SystemBase;


/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 */
class ArrayMooseVariable : public MooseVariableBase
{
public:
  ArrayMooseVariable(const std::string & name, unsigned int var_num, const FEType & fe_type, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind, unsigned int count);
  virtual ~ArrayMooseVariable();

  /**
   * Clear out the dof indices.  We do this in case this variable is not going to be prepared at all...
   */
  void clearDofIndices();

  virtual void prepare() override;

  virtual void prepareNeighbor() override;
  virtual void prepareAux() override;
  virtual void prepareIC() override;

  virtual void reinitNode() override;
  virtual void reinitNodeNeighbor() override;
  virtual void reinitAux() override;
  virtual void reinitAuxNeighbor() override;

  virtual void reinitNodes(const std::vector<dof_id_type> & nodes) override;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes) override;

  const std::set<SubdomainID> & activeSubdomains();

  /**
   * Is the variable active on the subdomain?
   * @param subdomain The subdomain id in question
   * @return true if active on subdomain, false otherwise
   */
  bool activeOnSubdomain(SubdomainID subdomain) const;

  /**
   * Is this variable nodal
   * @return true if it nodal, otherwise false
   */
  virtual bool isNodal() const override;

  /**
   * Current side this variable is being evaluated on
   */
  unsigned int & currentSide() { return _current_side; }

  /**
   * Current neighboring element
   */
  const Elem * & neighbor() { return _neighbor; }

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool computingSecond() { return _need_second || _need_second_old || _need_second_older; }

  const VariablePhiValue & phi();
  const ArrayVariablePhiGradient & gradPhi();
  const VariablePhiSecond & secondPhi();

  const VariablePhiValue & phiFace();
  const VariablePhiGradient & gradPhiFace();
  const VariablePhiSecond & secondPhiFace();

  const VariablePhiValue & phiNeighbor();
  const VariablePhiGradient & gradPhiNeighbor();
  const VariablePhiSecond & secondPhiNeighbor();

  const VariablePhiValue & phiFaceNeighbor();
  const VariablePhiGradient & gradPhiFaceNeighbor();
  const VariablePhiSecond & secondPhiFaceNeighbor();

  const MooseArray<Point> & normals() { return _normals; }

  // damping
  ArrayVariableValue & increment() { return _increment; }

  const ArrayVariableValue & sln() { return _u; }
  const ArrayVariableValue & slnOld() { _need_u_old = true; return _u_old; }
  const ArrayVariableValue & slnOlder() { _need_u_older = true;return _u_older; }
  const ArrayVariableGradient & gradSln() { return _grad_u; }
  const ArrayVariableGradient & gradSlnOld() { _need_grad_old = true; return _grad_u_old; }
  const ArrayVariableGradient & gradSlnOlder() { _need_grad_older = true; return _grad_u_older; }
  const ArrayVariableSecond & secondSln() { _need_second = true; secondPhi(); secondPhiFace(); return _second_u; }
  const ArrayVariableSecond & secondSlnOld() { _need_second_old = true; secondPhi(); secondPhiFace(); return _second_u_old; }
  const ArrayVariableSecond & secondSlnOlder() { _need_second_older = true; secondPhi(); secondPhiFace(); return _second_u_older; }

  const ArrayVariableValue & uDot() { return _u_dot; }
  const ArrayVariableValue & duDotDu() { return _du_dot_du; }

  const Node * & node() { return _node; }
  dof_id_type & nodalDofIndex() { return _nodal_dof_index; }
  bool isNodalDefined() { return _is_defined; }
  const ArrayVariableValue & nodalSln() { return _nodal_u; }
  const ArrayVariableValue & nodalSlnOld() { return _nodal_u_old; }
  const ArrayVariableValue & nodalSlnOlder() { return _nodal_u_older; }
  const ArrayVariableValue & nodalSlnDot() { return _nodal_u_dot; }
  const ArrayVariableValue & nodalSlnDuDotDu() { return _nodal_du_dot_du; }

  const ArrayVariableValue & nodalValue();
  const ArrayVariableValue & nodalValueOld();
  const ArrayVariableValue & nodalValueOlder();
  const ArrayVariableValue & nodalValueDot();

  const ArrayVariableValue & nodalValueNeighbor();
  const ArrayVariableValue & nodalValueOldNeighbor();
  const ArrayVariableValue & nodalValueOlderNeighbor();
  const ArrayVariableValue & nodalValueDotNeighbor();

  const ArrayVariableValue & slnNeighbor() { return _u_neighbor; }
  const ArrayVariableValue & slnOldNeighbor() { _need_u_old_neighbor = true; return _u_old_neighbor; }
  const ArrayVariableValue & slnOlderNeighbor() { _need_u_older_neighbor = true; return _u_older_neighbor; }
  const ArrayVariableGradient & gradSlnNeighbor() { return _grad_u_neighbor; }
  const ArrayVariableGradient & gradSlnOldNeighbor() { _need_grad_old_neighbor = true; return _grad_u_old_neighbor; }
  const ArrayVariableGradient & gradSlnOlderNeighbor() { _need_grad_older_neighbor = true; return _grad_u_older_neighbor; }
  const ArrayVariableSecond & secondSlnNeighbor() { _need_second_neighbor = true; secondPhiFaceNeighbor(); return _second_u_neighbor; }
  const ArrayVariableSecond & secondSlnOldNeighbor() { _need_second_old_neighbor = true; secondPhiFaceNeighbor(); return _second_u_old_neighbor; }
  const ArrayVariableSecond & secondSlnOlderNeighbor() { _need_second_older_neighbor = true; secondPhiFaceNeighbor(); return _second_u_older_neighbor; }

  const ArrayVariableValue & uDotNeighbor() { return _u_dot_neighbor; }
  const ArrayVariableValue & duDotDuNeighbor() { return _du_dot_du_neighbor; }

  const Node * & nodeNeighbor() { return _node_neighbor; }
  dof_id_type & nodalDofIndexNeighbor() { return _nodal_dof_index_neighbor; }
  bool isNodalNeighborDefined() { return _is_defined_neighbor; }
  const ArrayVariableValue & nodalSlnNeighbor() { return _nodal_u_neighbor; }
  const ArrayVariableValue & nodalSlnOldNeighbor() { return _nodal_u_old_neighbor; }
  const ArrayVariableValue & nodalSlnOlderNeighbor() { return _nodal_u_older_neighbor; }
  const ArrayVariableValue & nodalSlnDotNeighbor() { return _nodal_u_dot_neighbor; }
  const ArrayVariableValue & nodalSlnDuDotDuNeighbor() { return _nodal_du_dot_du_neighbor; }

  /**
   * Compute values at interior quadrature points
   */
  virtual void computeElemValues() override;
//  /**
//   * Compute values at facial quadrature points
//   */
//  void computeElemValuesFace();
//  /**
//   * Compute values at facial quadrature points for the neighbor
//   */
//  void computeNeighborValuesFace();
//  /**
//   * Compute values at quadrature points for the neighbor
//   */
//  void computeNeighborValues();
    /**
     * Compute nodal values of this variable
     */
    void computeNodalValues();
//  /**
//   * Compute nodal values of this variable in the neighbor
//   */
//  void computeNodalNeighborValues();
//  /**
//   * Set the nodal value for this variable to keep everything up to date
//   */
//  void setNodalValue(Number value, unsigned int idx = 0);
//  /**
//   * Set values for this variable to keep everything up to date
//   */
//  void setNodalValue(const DenseVector<Number> & value);
//
//  /**
//   * Set the neighbor nodal value for this variable
//   */
//  void setNodalValueNeighbor(Number value);
//  /**
//   * Set the neighbor values for this variable
//   */
//  void setNodalValueNeighbor(const DenseVector<Number> & value);
//
//  /**
//   * Compute and store incremental change based on increment_vec
//   */
//  void computeIncrement(const NumericVector<Number> & increment_vec);
//
//  /**
//   * Get DOF indices for currently selected element
//   * @return
//   */
//  std::vector<dof_id_type> & dofIndicesNeighbor() { return _dof_indices_neighbor; }
//
//  unsigned int numberOfDofsNeighbor() { return _dof_indices_neighbor.size(); }

//  void insert(NumericVector<Number> & residual);
//  void add(NumericVector<Number> & residual);

//  /**
//   * Get the value of this variable at given node
//   */
//  DenseVector getNodalValue(const Node & node);
//  /**
//   * Get the old value of this variable at given node
//   */
//  DenseVector getNodalValueOld(const Node & node);
//  /**
//   * Get the t-2 value of this variable at given node
//   */
//  DenseVector getNodalValueOlder(const Node & node);

  /**
   * Compute the variable value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable value
   */
  DenseVector<Real> getValue(const Elem * elem, const std::vector<std::vector<Real> > & phi) const;
  DenseMatrix<Real> getGradient(const Elem * elem, const std::vector<std::vector<RealGradient> > & phi) const;

  /**
   * Retrieve the Elemental DOF
   * @param elem The element we are computing on
   * @return The variable value
   */
  Number getElementalValue(const Elem * elem, unsigned int idx = 0) const;

  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhi() { return true; }

  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhi() { return true; }

  /**
   * Whether or not this variable is actually using the shape function second derivative.
   */
  bool usesSecondPhi() { return _need_second || _need_second_old || _need_second_older; }

protected:
  /**
   * Get dof indices for the variable
   * @param elem Element whose DOFs we are requesting (input)
   * @param dof_indices DOF indices for the given element (output)
   */
  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices);

protected:
  /// Thread ID
  THREAD_ID _tid;

  /// Quadrature rule for interior
  QBase * & _qrule;
  /// Quadrature rule for the face
  QBase * & _qrule_face;
  /// Quadrature rule for the neighbor
  QBase * & _qrule_neighbor;

  /// the side of the current element (valid when doing face assembly)
  unsigned int & _current_side;

  /// neighboring element
  const Elem * & _neighbor;

  /// DOF indices (neighbor)
  std::vector<dof_id_type> _dof_indices_neighbor;

  /// Cache of local dof indices for the first variable in the variable group
  std::vector<dof_id_type> _local_dof_indices;

  /// Interface to raw PetscVector data
  Eigen::Map<Eigen::VectorXd> _mapped_values;

  // Interface to the current gradient for each shape function
  ArrayVariablePhiGradient _mapped_grad_phi;

  bool _need_u_old;
  bool _need_u_older;

  bool _need_grad_old;
  bool _need_grad_older;

  bool _need_second;
  bool _need_second_old;
  bool _need_second_older;


  bool _need_u_old_neighbor;
  bool _need_u_older_neighbor;

  bool _need_grad_old_neighbor;
  bool _need_grad_older_neighbor;

  bool _need_second_neighbor;
  bool _need_second_old_neighbor;
  bool _need_second_older_neighbor;

  bool _need_nodal_u;
  bool _need_nodal_u_old;
  bool _need_nodal_u_older;
  bool _need_nodal_u_dot;

  bool _need_nodal_u_neighbor;
  bool _need_nodal_u_old_neighbor;
  bool _need_nodal_u_older_neighbor;
  bool _need_nodal_u_dot_neighbor;

  // Shape function values, gradients. second derivatives
  const VariablePhiValue & _phi;
  const VariablePhiGradient & _grad_phi;
  const VariablePhiSecond * _second_phi;

  // Values, gradients and second derivatives of shape function on faces
  const VariablePhiValue & _phi_face;
  const VariablePhiGradient & _grad_phi_face;
  const VariablePhiSecond * _second_phi_face;

 // Values, gradients and second derivatives of shape function
  const VariablePhiValue & _phi_neighbor;
  const VariablePhiGradient & _grad_phi_neighbor;
  const VariablePhiSecond * _second_phi_neighbor;

  // Values, gradients and second derivatives of shape function on faces
  const VariablePhiValue & _phi_face_neighbor;
  const VariablePhiGradient & _grad_phi_face_neighbor;
  const VariablePhiSecond * _second_phi_face_neighbor;

  /// Normals at QPs on faces
  const MooseArray<Point> & _normals;

  ArrayVariableValue _u, _u_bak;
  ArrayVariableValue _u_old, _u_old_bak;
  ArrayVariableValue _u_older, _u_older_bak;
  ArrayVariableGradient  _grad_u, _grad_u_bak;
  ArrayVariableGradient  _grad_u_old, _grad_u_old_bak;
  ArrayVariableGradient  _grad_u_older, _grad_u_older_bak;
  ArrayVariableSecond _second_u, _second_u_bak;
  ArrayVariableSecond _second_u_old, _second_u_old_bak;
  ArrayVariableSecond _second_u_older, _second_u_older_bak;

  ArrayVariableValue _u_neighbor;
  ArrayVariableValue _u_old_neighbor;
  ArrayVariableValue _u_older_neighbor;
  ArrayVariableGradient _grad_u_neighbor;
  ArrayVariableGradient _grad_u_old_neighbor;
  ArrayVariableGradient _grad_u_older_neighbor;
  ArrayVariableSecond _second_u_neighbor;
  ArrayVariableSecond _second_u_old_neighbor;
  ArrayVariableSecond _second_u_older_neighbor;

  // time derivatives

  /// u_dot (time derivative)
  ArrayVariableValue _u_dot, _u_dot_bak;
  ArrayVariableValue _u_dot_neighbor, _u_dot_bak_neighbor;

  /// derivative of u_dot wrt u
  ArrayVariableValue _du_dot_du, _du_dot_du_bak;
  ArrayVariableValue _du_dot_du_neighbor, _du_dot_du_bak_neighbor;

  // nodal stuff

  /// If the variable is defined at the node (used in compute nodal values)
  bool _is_defined;
  /// If true, the nodal value gets inserted on calling insert()
  bool _has_nodal_value;
  bool _has_nodal_value_neighbor;
  const Node * & _node;
  dof_id_type _nodal_dof_index;
  ArrayVariableValue _nodal_u;
  ArrayVariableValue _nodal_u_old;
  ArrayVariableValue _nodal_u_older;

  /// nodal values of u_dot
  ArrayVariableValue _nodal_u_dot;
  /// nodal values of derivative of u_dot wrt u
  ArrayVariableValue _nodal_du_dot_du;

  /// If the variable is defined at the neighbor node (used in compute nodal values)
  bool _is_defined_neighbor;
  const Node * & _node_neighbor;
  dof_id_type _nodal_dof_index_neighbor;
  ArrayVariableValue _nodal_u_neighbor;
  ArrayVariableValue _nodal_u_old_neighbor;
  ArrayVariableValue _nodal_u_older_neighbor;
  ArrayVariableValue _nodal_u_dot_neighbor;
  ArrayVariableValue _nodal_du_dot_du_neighbor;

  /// if variable is nodal
  bool _is_nodal;

  // damping
  ArrayVariableValue _increment;

  friend class NodeFaceConstraint;
  friend class ValueThresholdMarker;
  friend class ValueRangeMarker;
};

#endif /* ARRAYMOOSEVARIABLE_H */
