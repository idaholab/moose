/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeEigenstrainBase.h"
#include "RankTwoTensor.h"

// libmesh includes
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

class Assembly;
class InputParameters;
class MooseObject;
class SubProblem;

class ComputeReducedOrderEigenstrain : public ComputeEigenstrainBase
{
public:
  ComputeReducedOrderEigenstrain(const InputParameters & parameters);

  void initQpStatefulProperties();
  void computeProperties();
  void computeQpEigenstrain();

private:
  /// Subtract adjusted eigenstrain from strain
  void applyEigenstrain(MaterialProperty<RankTwoTensor> & strain);

  /// Add contributions from every eigenstrain at each integration point
  void sumEigenstrain();

  /// Compute either the volume average or linear eigenstrain field in an element
  void prepareEigenstrain();

  std::vector<MaterialPropertyName> _input_eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;

  SubProblem & _subproblem;
  /// Number of columns in A matrix (1 plus mesh dimension)
  const unsigned _ncols;
  /// Whether the mesh is made of second order elements
  const bool _second_order;
  /// The sum of all eigenstrains at each integration point
  std::vector<RankTwoTensor> _eigsum;
  /// The (num q points x ncols) array for the least squares.  Holds 1, xcoor, ycoor, zcoor.
  DenseMatrix<Real> _A;
  /// The b array holding the unique eigenstrain components for each integration point.
  std::vector<DenseVector<Real>> _b;
  /// Transpose of A
  DenseMatrix<Real> _AT;
  /// Transpose of A times b.
  DenseVector<Real> _ATb;
  /// The solution vector for each unique component of the adjusted eigenstrain
  std::vector<DenseVector<Real>> _x;
  /// Vector to hold the adjusted strain as computed with _x
  std::vector<Real> _vals;
  /// Filled with _vals and subracted from strain
  RankTwoTensor _adjusted_eigenstrain;
};

template <>
InputParameters validParams<ComputeReducedOrderEigenstrain>();
