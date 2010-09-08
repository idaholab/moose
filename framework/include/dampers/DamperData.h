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

#ifndef DAMPERDATA_H
#define DAMPERDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"
#include "ElementData.h"

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
class DamperData
{
public:
  DamperData(MooseSystem & moose_system, ElementData & element_data);

  ~DamperData();

  void init();

  /**
   * Computes the value of the increment for each variable at the quadrature points.
   */
  void reinit(const NumericVector<Number>& increment_vec);
  
  /**
   * The MooseSystem
   */
  MooseSystem & _moose_system;

  MooseArray<MooseArray<Real> > _var_increments;

  /**
   * A reference to the element data class... so we can reuse that data.
   *
   * This also means that the element data needs to get reinit BEFORE damper data!
   */
  ElementData & _element_data;

  /**
   * Dof Maps for all the variables.
   */
  std::vector<std::vector<unsigned int> > & _var_dof_indices;

  /**
   * quadrature rule.
   */
  QGauss * & _qrule;

  /**
   * number of quadrature points for current element
   */
  unsigned int & _n_qpoints;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the quadrature points
   */
  std::set<unsigned int> & _var_nums;

  /**
   * Shape function.
   */
  std::map<FEType, const std::vector<std::vector<Real> > *> & _phi;
};

#endif //DAMPERDATA_H
