//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialConditionBase.h"

// libMesh
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/elem.h"

// forward declarations
class FEProblemBase;
class Assembly;

/**
 * This is a template class that implements the workhorse `compute` and `computeNodal` methods. The
 * former method is used for setting block initial conditions. It first projects the initial
 * condition field to nodes, then to edges, then to faces, then to interior dofs. The latter
 * `computeNodal` method sets dof values for boundary restricted initial conditions
 */
template <typename T>
class InitialConditionTempl : public InitialConditionBase
{
public:
  typedef typename OutputTools<T>::OutputShape ValueType;
  typedef typename OutputTools<T>::OutputGradient GradientType;
  typedef typename OutputTools<T>::OutputShapeGradient GradientShapeType;
  typedef typename OutputTools<T>::OutputData DataType;
  typedef FEGenericBase<ValueType> FEBaseType;

  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialConditionTempl(const InputParameters & parameters);

  virtual ~InitialConditionTempl();

  static InputParameters validParams();

  virtual MooseVariableFEBase & variable() override { return _var; }

  virtual void compute() override;

  virtual void computeNodal(const Point & p) override;

  /**
   * The value of the variable at a point.
   *
   * This must be overridden by derived classes.
   */
  virtual T value(const Point & p) = 0;

  /**
   * The gradient of the variable at a point.
   *
   * This is optional.  Note that if you are using C1 continuous elements you will
   * want to use an initial condition that defines this!
   */
  virtual GradientType gradient(const Point & /*p*/) { return GradientType(); };

  T gradientComponent(GradientType grad, unsigned int i);

  /**
   * set the temporary solution vector for node projections of C0 variables
   */
  void setCZeroVertices();
  /**
   * set the temporary solution vector for node projections of Hermite variables
   */
  void setHermiteVertices();
  /**
   * set the temporary solution vector for node projections of non-Hermitian C1 variables
   */
  void setOtherCOneVertices();

  /**
   * Helps perform multiplication of GradientTypes: a normal dot product for vectors and a
   * contraction for tensors
   */
  Real dotHelper(const RealGradient & op1, const RealGradient & op2) { return op1 * op2; }
  Real dotHelper(const RealTensor & op1, const RealTensor & op2) { return op1.contract(op2); }
  RealEigenVector dotHelper(const RealVectorArrayValue & op1, const RealGradient & op2)
  {
    RealEigenVector v = op1.col(0) * op2(0);
    for (const auto i : make_range(Moose::dim))
      v += op1.col(i) * op2(i);
    return v;
  }

  /**
   * Perform the cholesky solves for edge, side, and interior projections
   */
  void choleskySolve(bool is_volume);

  /**
   * Assemble a small local system for cholesky solve
   */
  void choleskyAssembly(bool is_volume);

protected:
  FEProblemBase & _fe_problem;
  THREAD_ID _tid;

  /// Time
  Real & _t;

  /// The variable that this initial condition is acting upon.
  MooseVariableField<T> & _var;
  MooseVariableFE<T> * _fe_var;

  /// the finite element/volume assembly object
  Assembly & _assembly;

  /// The coordinate system type for this problem, references the value in Assembly
  const Moose::CoordinateSystemType & _coord_sys;

  /// The current element we are on will retrieving values at specific points in the domain. Note
  /// that this _IS_ valid even for nodes shared among several elements.
  const Elem * const & _current_elem;

  /// the volume of the current element
  const Real & _current_elem_volume;

  /// The current node if the point we are evaluating at also happens to be a node.
  /// Otherwise the pointer will be NULL.
  const Node * _current_node;

  /// The current quadrature point, contains the "nth" node number when visiting nodes.
  unsigned int _qp;

  /// Matrix storage member
  DenseMatrix<Real> _Ke;
  /// Linear b vector
  DenseVector<DataType> _Fe;
  /// Linear solution vector
  DenseVector<DataType> _Ue;

  /// The finite element type for the IC variable
  const FEType & _fe_type;
  /// The type of continuity, e.g. C0, C1
  FEContinuity _cont;

  /// The global DOF indices
  std::vector<dof_id_type> _dof_indices;
  /// Side/edge DOF indices
  std::vector<unsigned int> _side_dofs;

  /// The number of quadrature points for a given element
  unsigned int _n_qp;
  /// The number of nodes for a given element
  unsigned int _n_nodes;

  /// Whether the degree of freedom is fixed (true/false)
  std::vector<char> _dof_is_fixed;
  /// Stores the ids of the free dofs
  std::vector<int> _free_dof;

  /// The number of free dofs
  dof_id_type _free_dofs;
  /// The current dof being operated on
  dof_id_type _current_dof;
  /// number of dofs per node per variable
  dof_id_type _nc;

  /// pointers to shape functions
  const std::vector<std::vector<ValueType>> * _phi;
  /// pointers to shape function gradients
  const std::vector<std::vector<GradientShapeType>> * _dphi;
  /// pointers to the Jacobian * quadrature weights for current element
  const std::vector<Real> * _JxW;
  /// pointers to the xyz coordinates of the quadrature points for the current element
  const std::vector<Point> * _xyz_values;

  /// node counter
  unsigned int _n;
  /// the mesh dimension
  unsigned int _dim;
};

template <typename T>
InputParameters
InitialConditionTempl<T>::validParams()
{
  return InitialConditionBase::validParams();
}

typedef InitialConditionTempl<Real> InitialCondition;
typedef InitialConditionTempl<RealVectorValue> VectorInitialCondition;
typedef InitialConditionTempl<RealEigenVector> ArrayInitialCondition;
