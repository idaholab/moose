#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "Array.h"
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


typedef std::vector<Real>               VariableValue;
typedef std::vector<RealGradient>       VariableGradient;


namespace Moose
{

class AssemblyData;
class ProblemInterface;
class System;

#if 0
class VariableData
{
public:
  VariableData(THREAD_ID tid, const FEType & fe_type, System & sys);

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
  System & _sys;
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

  // time derivatives
  VariableValue _u_dot;
  VariableValue _du_dot_du;
};
#endif

class Variable
{
public:
  Variable(unsigned int var_num, const FEType & fe_type, System & sys, AssemblyData & assembly_data);
  virtual ~Variable();

  void reinit();
  void reinit_node();
  void reinit_aux();

  /**
   * Get variable number
   */
  unsigned int number() { return _var_num; }
//  int dimension() { return _dim; }

  const FEType feType() { return _fe->get_fe_type(); }

  const Elem * & currentElem() { return _elem; }
  unsigned int & currentSide() { return _current_side; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  std::vector<std::vector<Real> > & test() { return _test; }
  std::vector<std::vector<RealGradient> > & gradTest() { return _grad_test; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  std::vector<std::vector<Real> > & testFace() { return _test_face; }
  std::vector<std::vector<RealGradient> > & gradTestFace() { return _grad_test_face; }
  const std::vector<Point> & normals() { return _normals; }

  VariableValue & sln() { return _u; }
  VariableValue & slnOld() { return _u_old; }
  VariableValue & slnOlder() { return _u_older; }
  VariableGradient  & gradSln() { return _grad_u; }
  VariableGradient  & gradSlnOld() { return _grad_u_old; }
  VariableGradient  & gradSlnOlder() { return _grad_u_older; }

  VariableValue & uDot() { return _u_dot; }
  VariableValue & duDotDu() { return _du_dot_du; }

  const Node * & node() { return _node; }
  unsigned int & nodalDofIndex() { return _nodal_dof_index; }
  VariableValue & nodalSln() { return _nodal_u; }

  void computeElemValues();
  void computeElemValuesFace();

  void computeNodalValues();

  void sizeResidual();
  void sizeJacobianBlock();

  std::vector<unsigned int> & dofIndices() { return _dof_indices; }

  DenseVector<Number> & residualBlock() { return _Re; }
  DenseMatrix<Number> & jacobianBlock() { return _Ke; }

  void add(NumericVector<Number> & residual);
  void add(SparseMatrix<Number> & jacobian);

  Number getNodalValue(const Node & node);

protected:
  THREAD_ID _tid;
  unsigned int _var_num;
  ProblemInterface & _problem;
  System & _sys;

  const DofMap & _dof_map;
  AssemblyData & _assembly;

  QBase * & _qrule;
  QBase * & _qrule_face;

  FEBase * & _fe;
  FEBase * & _fe_face;

  const Elem * & _elem;
  unsigned int & _current_side;

  std::vector<unsigned int> _dof_indices;

//  const std::vector<Point> & _qpoints;
//  const std::vector<Point> & _qpoints_face;

//  const std::vector<Real> & _JxW;
//  const std::vector<Real> & _JxW_face;

  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  std::vector<std::vector<Real> > _test;
  std::vector<std::vector<RealGradient> > _grad_test;

  const std::vector<std::vector<Real> > & _phi_face;
  const std::vector<std::vector<RealGradient> > & _grad_phi_face;
  std::vector<std::vector<Real> > _test_face;
  std::vector<std::vector<RealGradient> > _grad_test_face;
  const std::vector<Point> & _normals;

  VariableValue _u;
  VariableValue _u_old;
  VariableValue _u_older;
  VariableGradient  _grad_u;
  VariableGradient  _grad_u_old;
  VariableGradient  _grad_u_older;

  // time derivatives
  VariableValue _u_dot;
  VariableValue _du_dot_du;

  // nodal stuff
  const Node * & _node;
  unsigned int _nodal_dof_index;
  VariableValue _nodal_u;
  VariableGradient  _nodal_grad_u;

  /**
   * Residual for this variable
   */
  DenseVector<Number> _Re;
  DenseMatrix<Number> _Ke;
};

}

#endif /* VARIABLE_H_ */
