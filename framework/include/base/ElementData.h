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

  void reinitKernels(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke);

  void reinitMaterials(std::vector<Material *> & materials);

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
};

#endif //ELEMENTDATA_H
