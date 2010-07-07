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
  ElementData(MooseSystem & moose_system);

  ~ElementData();

  void sizeEverything();

  void init();

  void initKernels();

  void reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke);
  
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
  std::vector<std::map<unsigned int, std::vector<std::vector<Real> > > > _test;

  /**
   * Current element
   */
  std::vector<const Elem *> _current_elem;

    /**
   * Residual vectors for all variables.
   */
  std::vector<std::vector<DenseSubVector<Number> * > > _var_Res;

  /**
   * Jacobian matrices for all variables.
   */
  std::vector<std::vector<DenseMatrix<Number> * > > _var_Kes;

};

#endif //ELEMENTDATA_H
