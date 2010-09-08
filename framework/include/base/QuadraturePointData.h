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

#ifndef QUADRPTDATA_H
#define QUADRPTDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"

//Forward Declarations
class MooseSystem;
class Material;
class DofData;

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
 * Holds quadrature point related data
 */
class QuadraturePointData
{
public:
  QuadraturePointData(MooseSystem & moose_system, DofData & dof_data);
  virtual ~QuadraturePointData();

  void init();

  void reinit(const NumericVector<Number>& soln, const Elem * elem);

  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Reference to DofData
   */
  DofData & _dof_data;

  /**
   * finite element.
   */
  std::map<FEType, FEBase *> _fe;

  /**
   * finite element for the displaced mesh.
   */
  std::map<FEType, FEBase *> _fe_displaced;
  
  /**
   * quadrature rule.
   */
  QGauss * _qrule;

  /**
   * number of quadrature points for current element
   */
  unsigned int _n_qpoints;
  
  /**
   * XYZ coordinates of quadrature points
   */
  std::map<FEType, const std::vector<Point> *> _q_point;

  /**
   * XYZ coordinates of the displaced quadrature points
   */
  std::map<FEType, const std::vector<Point> *> _q_point_displaced;

  /**
   * Jacobian pre-multiplied by the weight.
   */
  std::map<FEType, const std::vector<Real> *> _JxW;

  /**
   * Jacobian pre-multiplied by the weight for the displaced mesh.
   */
  std::map<FEType, const std::vector<Real> *> _JxW_displaced;
  
  /**
   * Shape function.
   */
  std::map<FEType, const std::vector<std::vector<Real> > *> _phi;

  /**
   * Gradient of shape function.
   */
  std::map<FEType, const std::vector<std::vector<RealGradient> > *> _grad_phi;

  /**
   * Second derivative of interior shape function.
   */
  std::map<FEType, const std::vector<std::vector<RealTensor> > *> _second_phi;


  /**
   * Vector of variable numbers that need to be evaluated
   * at the quadrature points
   */
  std::set<unsigned int> _var_nums;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<Real> > _var_vals;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<Real> > _var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  MooseArray<MooseArray<Real> > _var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _var_grads;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _var_grads_older;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealTensor> > _var_seconds;


  /**
   * Vector of auxiliary variable numbers that need to be evaluated
   * at the quadrature points
   */
  std::set<unsigned int> _aux_var_nums;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<Real> > _aux_var_vals;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<Real> > _aux_var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  MooseArray<MooseArray<Real> > _aux_var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _aux_var_grads;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _aux_var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<RealGradient> > _aux_var_grads_older;

  /**
   * Pointer to the material that is valid for the current block.
   */
  std::vector<Material *> _material;


  /**
   * Value of the variables at the quadrature points at previous newton step.
   */
  MooseArray<MooseArray<Real> > _var_vals_old_newton;

  /**
   * Gradient of the variables at the quadrature points at previous newton step.
   */
  MooseArray<MooseArray<RealGradient> > _var_grads_old_newton;
};

#endif // QUADRPTDATA_H
