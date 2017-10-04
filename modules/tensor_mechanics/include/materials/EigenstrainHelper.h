/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaterialProperty.h"
#include "RankTwoTensor.h"

// libmesh includes
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/quadrature.h"

class Assembly;
class InputParameters;
class MooseObject;
class SubProblem;

class EigenstrainHelper
{
public:
  EigenstrainHelper(const MooseObject * moose_object, bool bnd);

  void applyIncrementalEigenstrain(
      const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
      const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains_old,
      MaterialProperty<RankTwoTensor> & strain);

  void
  applyTotalEigenstrain(const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
                        MaterialProperty<RankTwoTensor> & strain);

private:
  /// Subtract adjusted eigenstrain from strain
  void applyEigenstrain(MaterialProperty<RankTwoTensor> & strain);

  /// Add incremental strain from every eigenstrain at each integration point
  void sumEigenstrainIncremental(
      const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains,
      const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains_old);

  /// Add contributions from every eigenstrain at each integration point
  void
  sumEigenstrainTotal(const std::vector<const MaterialProperty<RankTwoTensor> *> & eigenstrains);

  /// Compute either the volume average or linear eigenstrain field in an element
  void prepareEigenstrain();

  const InputParameters & _eh_params;
  SubProblem & _eh_subproblem;
  const THREAD_ID _eh_tid;
  Assembly & _eh_assembly;
  QBase *& _eh_qrule;
  const MooseArray<Real> & _eh_JxW;
  const MooseArray<Real> & _eh_coord;
  const MooseArray<Point> & _eh_q_point;
  /// Number of columns in A matrix (1 plus mesh dimension)
  const unsigned _eh_ncols;
  /// Whether the mesh is made of second order elements
  const bool _eh_second_order;
  /// The sum of all eigenstrains at each integration point
  std::vector<RankTwoTensor> _eh_eigsum;
  /// The (num q points x ncols) array for the least squares.  Holds 1, xcoor, ycoor, zcoor.
  DenseMatrix<Real> _eh_A;
  /// The b array holding the unique eigenstrain components for each integration point.
  std::vector<DenseVector<Real>> _eh_b;
  /// Transpose of A
  DenseMatrix<Real> _eh_AT;
  /// Traspose of A times b.
  DenseVector<Real> _eh_ATb;
  /// The solution vector for each unique component of the adjusted eigenstrain
  std::vector<DenseVector<Real>> _eh_x;
  /// Vector to hold the adjusted strain as computed with _eh_x
  std::vector<Real> _eh_vals;
  /// Filled with _eh_vals and subracted from strain
  RankTwoTensor _eh_adjusted_eigenstrain;
};
