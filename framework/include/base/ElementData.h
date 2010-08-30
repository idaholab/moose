#ifndef ELEMENTDATA_H
#define ELEMENTDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"
#include "QuadraturePointData.h"

//Forward Declarations
class MooseSystem;

namespace libMesh
{
  class QGauss;
  class DofMap;
  class FEBase;
  template<class T> class NumericVector;
  template<class T> class DenseVector;
  template<class T> class DenseSubVector;
  template<class T> class DenseMatrix;
}

/**
 * One stop shop for all the data a Kernel class needs.
 *
 * _One_ of these will get built for each MooseSystem.
 */
class ElementData : public QuadraturePointData
{
public:
  ElementData(MooseSystem & moose_system, DofData & dof_data);

  ~ElementData();

  void init();

  void initKernels();

  void reinitKernels(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke);

  /**
   * Stores the values of variables for actual newton step
   */
  void reinitNewtonStep(const NumericVector<Number>& soln);

  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Interior test function.
   *
   * Note that there is a different test function for each variable... allowing for modified
   * basis for things like SUPG and GLS.
   */
  std::map<unsigned int, std::vector<std::vector<Real> > > _test;

  /**
   * Value of the variables at the quadrature points at previous newton step.
   */
  MooseArray<MooseArray<Real> > _var_vals_old_newton;

  /**
   * Gradient of the variables at the quadrature points at previous newton step.
   */
  MooseArray<MooseArray<RealGradient> > _var_grads_old_newton;
};

#endif //ELEMENTDATA_H
