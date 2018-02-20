//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEVARIABLEFE_H
#define MOOSEVARIABLEFE_H

#include "MooseVariableBase.h"

#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"

class Assembly;

class MooseVariableFE : public MooseVariableBase
{
public:
  MooseVariableFE(unsigned int var_num,
                  const FEType & fe_type,
                  SystemBase & sys,
                  Moose::VarKindType var_kind,
                  Assembly & assembly);

  virtual ~MooseVariableFE();

  /**
   * Clear out the dof indices.  We do this in case this variable is not going to be prepared at
   * all...
   */
  void clearDofIndices();

  void prepare();

  void prepareNeighbor();
  void prepareAux();

  void reinitNode();
  void reinitNodeNeighbor();
  void reinitAux();
  void reinitAuxNeighbor();

  void reinitNodes(const std::vector<dof_id_type> & nodes);
  void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes);

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool computingSecond()
  {
    return _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  }
  bool computingCurl() { return _need_curl || _need_curl_old; }

  const std::set<SubdomainID> & activeSubdomains() const;

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
  bool isNodal() const { return _is_nodal; }

  /**
   * Current element this variable is evaluated at
   */
  const Elem *& currentElem() const { return _elem; }

  /**
   * Current side this variable is being evaluated on
   */
  unsigned int & currentSide() const { return _current_side; }

  /**
   * Current neighboring element
   */
  const Elem *& neighbor() const { return _neighbor; }

  const MooseArray<Point> & normals() const { return _normals; }

  const Node *& node() const { return _node; }
  dof_id_type & nodalDofIndex() { return _nodal_dof_index; }
  bool isNodalDefined() const { return _has_dofs; }

  const Node *& nodeNeighbor() const { return _node_neighbor; }
  dof_id_type & nodalDofIndexNeighbor() { return _nodal_dof_index_neighbor; }
  bool isNodalNeighborDefined() const { return _neighbor_has_dofs; }

  const DenseVector<Number> & solutionDoFs()
  {
    _need_solution_dofs = true;
    return _solution_dofs;
  }
  const DenseVector<Number> & solutionDoFsOld()
  {
    _need_solution_dofs_old = true;
    return _solution_dofs_old;
  }
  const DenseVector<Number> & solutionDoFsOlder()
  {
    _need_solution_dofs_older = true;
    return _solution_dofs_older;
  }
  const DenseVector<Number> & solutionDoFsNeighbor()
  {
    _need_solution_dofs_neighbor = true;
    return _solution_dofs_neighbor;
  }
  const DenseVector<Number> & solutionDoFsOldNeighbor()
  {
    _need_solution_dofs_old_neighbor = true;
    return _solution_dofs_old_neighbor;
  }
  const DenseVector<Number> & solutionDoFsOlderNeighbor()
  {
    _need_solution_dofs_older_neighbor = true;
    return _solution_dofs_older_neighbor;
  }

  const MooseArray<Number> & nodalValue();
  const MooseArray<Number> & nodalValueOld();
  const MooseArray<Number> & nodalValueOlder();
  const MooseArray<Number> & nodalValuePreviousNL();
  const MooseArray<Number> & nodalValueDot();
  const MooseArray<Number> & nodalValueDuDotDu();
  const MooseArray<Number> & nodalValueNeighbor();
  const MooseArray<Number> & nodalValueOldNeighbor();
  const MooseArray<Number> & nodalValueOlderNeighbor();
  const MooseArray<Number> & nodalValuePreviousNLNeighbor();
  const MooseArray<Number> & nodalValueDotNeighbor();
  const MooseArray<Number> & nodalValueDuDotDuNeighbor();

  /**
   * Compute values at interior quadrature points
   */
  virtual void computeElemValues() = 0;
  /**
   * Compute values at facial quadrature points
   */
  virtual void computeElemValuesFace() = 0;
  /**
   * Compute values at facial quadrature points for the neighbor
   */
  virtual void computeNeighborValuesFace() = 0;
  /**
   * Compute values at quadrature points for the neighbor
   */
  virtual void computeNeighborValues() = 0;
  /**
   * Compute nodal values of this variable
   */
  virtual void computeNodalValues();
  /**
   * Compute nodal values of this variable in the neighbor
   */
  virtual void computeNodalNeighborValues();
  /**
   * Set the nodal value for this variable to keep everything up to date
   */
  virtual void setNodalValue(Number value, unsigned int idx = 0) = 0;
  /**
   * Set values for this variable to keep everything up to date
   */
  virtual void setNodalValue(const DenseVector<Number> & value) = 0;
  /**
   * Get the value of this variable at given node
   */
  Number getNodalValue(const Node & node);
  /**
   * Get the old value of this variable at given node
   */
  Number getNodalValueOld(const Node & node);
  /**
   * Get the t-2 value of this variable at given node
   */
  Number getNodalValueOlder(const Node & node);

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

  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhiNeighbor() { return true; }

  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhiNeighbor() { return true; }

  /**
   * Whether or not this variable is actually using the shape function second derivative on a
   * neighbor.
   */
  bool usesSecondPhiNeighbor()
  {
    return _need_second_neighbor || _need_second_old_neighbor || _need_second_older_neighbor;
  }

  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices);
  /**
   * Get neighbor DOF indices for currently selected element
   * @return the neighbor degree of freedom indices
   */
  std::vector<dof_id_type> & dofIndicesNeighbor() { return _dof_indices_neighbor; }

  unsigned int numberOfDofsNeighbor() { return _dof_indices_neighbor.size(); }

  void insert(NumericVector<Number> & residual);
  void add(NumericVector<Number> & residual);

  /// Returns dof solution on element
  const MooseArray<Number> & dofValue();

protected:
  /// Our assembly
  Assembly & _assembly;

  /// Thread ID
  THREAD_ID _tid;

  /// Quadrature rule for interior
  QBase *& _qrule;
  /// Quadrature rule for the face
  QBase *& _qrule_face;
  /// Quadrature rule for the neighbor
  QBase *& _qrule_neighbor;

  /// current element
  const Elem *& _elem;
  /// the side of the current element (valid when doing face assembly)
  unsigned int & _current_side;

  /// neighboring element
  const Elem *& _neighbor;

  /// DOF indices (neighbor)
  std::vector<dof_id_type> _dof_indices_neighbor;

  bool _need_u_old;
  bool _need_u_older;
  bool _need_u_previous_nl;

  bool _need_grad_old;
  bool _need_grad_older;
  bool _need_grad_previous_nl;

  bool _need_second;
  bool _need_second_old;
  bool _need_second_older;
  bool _need_second_previous_nl;

  bool _need_curl;
  bool _need_curl_old;
  bool _need_curl_older;

  bool _need_u_old_neighbor;
  bool _need_u_older_neighbor;
  bool _need_u_previous_nl_neighbor;

  bool _need_grad_old_neighbor;
  bool _need_grad_older_neighbor;
  bool _need_grad_previous_nl_neighbor;

  bool _need_second_neighbor;
  bool _need_second_old_neighbor;
  bool _need_second_older_neighbor;
  bool _need_second_previous_nl_neighbor;

  bool _need_curl_neighbor;
  bool _need_curl_old_neighbor;
  bool _need_curl_older_neighbor;

  bool _need_solution_dofs;
  bool _need_solution_dofs_old;
  bool _need_solution_dofs_older;
  bool _need_solution_dofs_neighbor;
  bool _need_solution_dofs_old_neighbor;
  bool _need_solution_dofs_older_neighbor;

  bool _need_dof_u;
  bool _need_dof_u_old;
  bool _need_dof_u_older;
  bool _need_dof_u_previous_nl;
  bool _need_dof_u_dot;
  bool _need_dof_du_dot_du;
  bool _need_dof_u_neighbor;
  bool _need_dof_u_old_neighbor;
  bool _need_dof_u_older_neighbor;
  bool _need_dof_u_previous_nl_neighbor;
  bool _need_dof_u_dot_neighbor;
  bool _need_dof_du_dot_du_neighbor;

  /// Normals at QPs on faces
  const MooseArray<Point> & _normals;

  /// if variable is nodal
  bool _is_nodal;
  /// If we have dofs
  bool _has_dofs;
  /// If the neighor has dofs
  bool _neighbor_has_dofs;

  /// If true, the nodal value gets inserted on calling insert()
  bool _has_nodal_value;
  bool _has_nodal_value_neighbor;

  const Node *& _node;
  const Node *& _node_neighbor;

  dof_id_type _nodal_dof_index;
  dof_id_type _nodal_dof_index_neighbor;

  // dof solution stuff (which for nodal variables corresponds to values at the nodes)

  MooseArray<Real> _dof_u;
  MooseArray<Real> _dof_u_old;
  MooseArray<Real> _dof_u_older;
  MooseArray<Real> _dof_u_previous_nl;

  /// nodal values of u_dot
  MooseArray<Real> _dof_u_dot;
  /// nodal values of derivative of u_dot wrt u
  MooseArray<Real> _dof_du_dot_du;

  MooseArray<Real> _dof_u_neighbor;
  MooseArray<Real> _dof_u_old_neighbor;
  MooseArray<Real> _dof_u_older_neighbor;
  MooseArray<Real> _dof_u_previous_nl_neighbor;
  MooseArray<Real> _dof_u_dot_neighbor;
  MooseArray<Real> _dof_du_dot_du_neighbor;

  /// local elemental DoFs
  DenseVector<Number> _solution_dofs;
  DenseVector<Number> _solution_dofs_old;
  DenseVector<Number> _solution_dofs_older;
  DenseVector<Number> _solution_dofs_neighbor;
  DenseVector<Number> _solution_dofs_old_neighbor;
  DenseVector<Number> _solution_dofs_older_neighbor;
};

#endif /* MOOSEVARIABLEFE_H */
