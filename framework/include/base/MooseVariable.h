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

#ifndef MOOSEVARIABLE_H
#define MOOSEVARIABLE_H

#include "MooseArray.h"
#include "ParallelUniqueId.h"

// libMesh
#include "fe.h"
#include "quadrature.h"
#include "dense_vector.h"
#include "dense_matrix.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"
#include "elem.h"
#include "node.h"


typedef MooseArray<Real>               VariableValue;
typedef MooseArray<RealGradient>       VariableGradient;
typedef MooseArray<RealTensor>         VariableSecond;


class AssemblyData;
class Problem;
class SystemBase;

#if 0
class VariableData
{
public:
  VariableData(THREAD_ID tid, const FEType & fe_type, SystemBase & sys);

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  std::vector<std::vector<Real> > & test() { return _test; }
  std::vector<std::vector<RealGradient> > & gradTest() { return _grad_test; }

  VariableValue & sln() { return _u; }
  VariableValue & slnOld() { return _u_old; }
  VariableValue & slnOlder() { return _u_older; }
  VariableGradient  & gradSln() { return _grad_u; }
  VariableGradient  & gradSlnOld() { return _grad_u_old; }
  VariableGradient  & gradSlnOlder() { return _grad_u_older; }

  VariableValue & uDot() { return _u_dot; }
  VariableValue & duDotDu() { return _du_dot_du; }

protected:

  void computeValues();

  ProblemInterface & _problem;
  SystemBase & _sys;
  FEBase * & _fe;
  QBase * & _qrule;

  std::vector<unsigned int> _dof_indices;

  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;

  std::vector<std::vector<Real> > _test;
  std::vector<std::vector<RealGradient> > _grad_test;

  VariableValue _u;
  VariableValue _u_old;
  VariableValue _u_older;
  VariableGradient  _grad_u;
  VariableGradient  _grad_u_old;
  VariableGradient  _grad_u_older;
  VariableValue _u_update;

  // time derivatives
  VariableValue _u_dot;
  VariableValue _du_dot_du;
};
#endif


/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 */
class MooseVariable
{
public:
  MooseVariable(unsigned int var_num, const FEType & fe_type, SystemBase & sys, AssemblyData & assembly_data);
  virtual ~MooseVariable();

  void prepare();
  void prepareNeighbor();
  void prepare_aux();
  void reinit_node();
  void reinit_aux();

  /// Get the variable number
  unsigned int number() { return _var_num; }
//  int dimension() { return _dim; }

  /// Get the type of finite element object
  const FEType feType() { return _fe->get_fe_type(); }

  /// Get the order of this variable
  Order getOrder() const { return _fe->get_order(); }

  /// is this variable nodal
  bool isNodal();

  // Read-only access to FE object used by this variable
  FEBase * const & currentFE() const { return _fe; }
  /// Current element this variable is evaluated at
  const Elem * & currentElem() { return _elem; }
  /// Current side this variable is being evaluated on
  unsigned int & currentSide() { return _current_side; }
  /// Current neighboring element
  const Elem * & neighbor() { return _neighbor; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  const std::vector<std::vector<RealTensor> > & secondPhi() { return _second_phi; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  const std::vector<std::vector<RealTensor> > & secondPhiFace() { return _second_phi_face; }

  const std::vector<std::vector<Real> > & phiFaceNeighbor() { return _phi_face_neighbor; }
  const std::vector<std::vector<RealGradient> > & gradPhiFaceNeighbor() { return _grad_phi_face_neighbor; }
  const std::vector<std::vector<RealTensor> > & secondPhiFaceNeighbor() { return _second_phi_face_neighbor; }

  const std::vector<Point> & normals() { return _normals; }

  // damping
  VariableValue & increment() { return _increment; }

  VariableValue & sln() { return _u; }
  VariableValue & slnOld() { return _u_old; }
  VariableValue & slnOlder() { return _u_older; }
  VariableGradient  & gradSln() { return _grad_u; }
  VariableGradient  & gradSlnOld() { return _grad_u_old; }
  VariableGradient  & gradSlnOlder() { return _grad_u_older; }
  VariableSecond & secondSln() { return _second_u; }
  VariableSecond & secondSlnOld() { return _second_u_old; }
  VariableSecond & secondSlnOlder() { return _second_u_older; }

  VariableValue & uDot() { return _u_dot; }
  VariableValue & duDotDu() { return _du_dot_du; }

  const Node * & node() { return _node; }
  unsigned int & nodalDofIndex() { return _nodal_dof_index; }
  VariableValue & nodalSln() { return _nodal_u; }
  VariableValue & nodalSlnOld() { return _nodal_u_old; }
  VariableValue & nodalSlnOlder() { return _nodal_u_older; }

  VariableValue & slnNeighbor() { return _u_neighbor; }
  VariableValue & slnOldNeighbor() { return _u_old_neighbor; }
  VariableValue & slnOlderNeighbor() { return _u_older_neighbor; }
  VariableGradient & gradSlnNeighbor() { return _grad_u_neighbor; }
  VariableGradient & gradSlnOldNeighbor() { return _grad_u_old_neighbor; }
  VariableGradient & gradSlnOlderNeighbor() { return _grad_u_older_neighbor; }
  VariableSecond & secondSlnNeighbor() { return _second_u_neighbor; }

  /// Compute values at interior quadrature points
  void computeElemValues();
  /// Compute values at facial quadrature points
  void computeElemValuesFace();
  /// Compute values at facial quadrature points for the neighbor
  void computeNeighborValuesFace();
  /// Compute nodal values of this variable
  void computeNodalValues();
  /// Set the nodal value for this variable (to keep everything up to date
  void setNodalValue(Number value);
  /// Compute damping for this variable based on increment_vec
  void computeDamping(const NumericVector<Number> & increment_vec);

  /// get DOF indices for currently selected element
  std::vector<unsigned int> & dofIndices() { return _dof_indices; }
  /// get DOF indices for currently selected element
  std::vector<unsigned int> & dofIndicesNeighbor() { return _dof_indices_neighbor; }

  void insert(NumericVector<Number> & residual);

  /// Get the value of this variable at given node
  Number getNodalValue(const Node & node);
  /// Get the old value of this variable at given node
  Number getNodalValueOld(const Node & node);
  /// Get the t-2 value of this variable at given node
  Number getNodalValueOlder(const Node & node);

  /// Set the scaling factor for this variable
  void scalingFactor(Real factor) { _scaling_factor = factor; }
  /// Get the scaling factor for this variable
  Real scalingFactor() { return _scaling_factor; }

protected:
  THREAD_ID _tid;                                               /// Thread ID
  unsigned int _var_num;                                        /// variable number (from libMesh)
  Problem & _problem;                                           /// Problem this variable is part of
  SystemBase & _sys;                                            /// System this variable is part of

  const DofMap & _dof_map;                                      /// DOF map
  AssemblyData & _assembly;                                     /// Assembly data

  QBase * & _qrule;                                             /// Quadrature rule for interior
  QBase * & _qrule_face;                                        /// Quadrature rule for the face

  FEBase * & _fe;                                               /// libMesh's FE object for this variable
  FEBase * & _fe_face;                                          /// libMesh's FE object for this variable on a face
  FEBase * & _fe_face_neighbor;                                 /// libMesh's FE object for this variable on a face on the neighboring element

  const Elem * & _elem;                                         /// current element
  unsigned int & _current_side;                                 /// the side of the current element (valid when doing face assembly)

  const Elem * & _neighbor;                                     /// neighboring element

  std::vector<unsigned int> _dof_indices;                       /// DOF indices
  std::vector<unsigned int> _dof_indices_neighbor;              /// DOF indices (neighbor)

  bool _is_nl;                                                  /// true if this varaible is non-linear
  bool _has_second_derivatives;                                 /// true if we need to compute second derivatives (if the variable is 3rd hermite, etc.)

  // Shape function values, gradients. second derivatives
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;

  // Values, gradients and second derivatives of shape function on faces
  const std::vector<std::vector<Real> > & _phi_face;
  const std::vector<std::vector<RealGradient> > & _grad_phi_face;
  const std::vector<std::vector<RealTensor> > & _second_phi_face;

  std::vector<std::vector<RealTensor> > _dummy_second_phi;            ///< if we are not using second derivatives, we point our reference here, so libmesh
                                                                      ///< will not compute second derivatives for us

  // Values, gradients and second derivatives of shape function on faces
  const std::vector<std::vector<Real> > & _phi_face_neighbor;
  const std::vector<std::vector<RealGradient> > & _grad_phi_face_neighbor;
  const std::vector<std::vector<RealTensor> > & _second_phi_face_neighbor;

  const std::vector<Point> & _normals;                                  ///< Normals at QPs on faces

  VariableValue _u;
  VariableValue _u_old;
  VariableValue _u_older;
  VariableGradient  _grad_u;
  VariableGradient  _grad_u_old;
  VariableGradient  _grad_u_older;
  VariableSecond _second_u;
  VariableSecond _second_u_old;
  VariableSecond _second_u_older;

  VariableValue _u_neighbor;
  VariableValue _u_old_neighbor;
  VariableValue _u_older_neighbor;
  VariableGradient _grad_u_neighbor;
  VariableGradient _grad_u_old_neighbor;
  VariableGradient _grad_u_older_neighbor;
  VariableSecond _second_u_neighbor;

  // time derivatives
  VariableValue _u_dot;                                                 ///< u_dot (time derivative)
  VariableValue _du_dot_du;                                             ///< derivative of u_dot wrt u

  // nodal stuff
  bool _is_defined;                                                     ///< If the variable is defined at the node (used in compute nodal values)
  bool _has_nodal_value;                                                ///< If true, the nodal value gets inserted on calling insert()
  const Node * & _node;
  unsigned int _nodal_dof_index;
  VariableValue _nodal_u;
  VariableValue _nodal_u_old;
  VariableValue _nodal_u_older;

  // damping
  VariableValue _increment;

  Real _scaling_factor;                                                 ///< scaling factor for this variable
};

#endif /* MOOSEVARIABLE_H */
