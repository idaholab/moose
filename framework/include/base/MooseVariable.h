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
  void prepare_aux();
  void reinit();
  void reinit_node();
  void reinit_aux();

  /// Get the variable number
  unsigned int number() { return _var_num; }
//  int dimension() { return _dim; }

  /// Get the type of finite element object
  const FEType feType() { return _fe->get_fe_type(); }

  /// is this variable nodal
  bool isNodal();

  /// Current element this variable is evaluated at
  const Elem * & currentElem() { return _elem; }
  /// Current side this varaible is being evalued on
  unsigned int & currentSide() { return _current_side; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  const std::vector<std::vector<RealTensor> > & secondPhi() { return _second_phi; }
  std::vector<std::vector<Real> > & test() { return _test; }
  std::vector<std::vector<RealGradient> > & gradTest() { return _grad_test; }
  std::vector<std::vector<RealTensor> > & secondTest() { return _second_test; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  const std::vector<std::vector<RealTensor> > & secondPhiFace() { return _second_phi_face; }
  std::vector<std::vector<Real> > & testFace() { return _test_face; }
  std::vector<std::vector<RealGradient> > & gradTestFace() { return _grad_test_face; }
  std::vector<std::vector<RealTensor> > & secondTestFace() { return _second_test_face; }
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

  /// Compute values at interior quadrature points
  void computeElemValues();
  /// COmpute values at facial quadrature points
  void computeElemValuesFace();
  /// Compute nodal values of this varaible
  void computeNodalValues();
  /// Set the nodal value for this variable (to keep everything up to date
  void setNodalValue(Number value);
  /// Store the value of aux variable
  void storeAuxValue(Number value);
  /// Compute damping for this variable based on increment_vec
  void computeDamping(const NumericVector<Number> & increment_vec);

  /// Size the dense vector that keeps the residual contributions
  void sizeResidual();
  /// Size the dense matrix that keeps the jacobian contributions
  void sizeJacobianBlock();
  /// get DOF indices for currently selectd element
  std::vector<unsigned int> & dofIndices() { return _dof_indices; }

  DenseVector<Number> & residualBlock() { return _Re; }
  DenseMatrix<Number> & jacobianBlock() { return _Ke; }

  /// Add the residual part into 'residual'
  void add(NumericVector<Number> & residual);
  /// Add the jacobian part into 'jacobian'
  void add(SparseMatrix<Number> & jacobian);

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

  const Elem * & _elem;                                         /// current element
  unsigned int & _current_side;                                 /// the side of the current element (valid when doing face assembly)

  std::vector<unsigned int> _dof_indices;                       /// DOF indices

  bool _is_nl;                                                  /// true if this varaible is non-linear
  bool _has_second_derivatives;                                 /// true if we need to compute second derivatives (if the variable is 3rd hermite, etc.)

//  const std::vector<Point> & _qpoints;
//  const std::vector<Point> & _qpoints_face;

//  const std::vector<Real> & _JxW;
//  const std::vector<Real> & _JxW_face;

  // Shape function values, gradients. second derivatives
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;
  // Test function values, gradients. second derivatives
  std::vector<std::vector<Real> > _test;
  std::vector<std::vector<RealGradient> > _grad_test;
  std::vector<std::vector<RealTensor> > _second_test;

  // Values, gradients and second derivatives of shape function on faces
  const std::vector<std::vector<Real> > & _phi_face;
  const std::vector<std::vector<RealGradient> > & _grad_phi_face;
  const std::vector<std::vector<RealTensor> > & _second_phi_face;
  // Values, gradients and second derivatives of test functions on faces
  std::vector<std::vector<Real> > _test_face;
  std::vector<std::vector<RealGradient> > _grad_test_face;
  std::vector<std::vector<RealTensor> > _second_test_face;

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

  // time derivatives
  VariableValue _u_dot;                                                 ///< u_dot (time derivative)
  VariableValue _du_dot_du;                                             ///< derivative of u_dot wrt u

  // nodal stuff
  const Node * & _node;
  unsigned int _nodal_dof_index;
  VariableValue _nodal_u;
  VariableValue _nodal_u_old;
  VariableValue _nodal_u_older;

  // damping
  VariableValue _increment;

  DenseVector<Number> _Re;                                              ///< Residual for this variable
  DenseMatrix<Number> _Ke;                                              ///< Jacobian for this variable

  Real _scaling_factor;                                                 ///< scaling factor for this variable
};

#endif /* MOOSEVARIABLE_H */
