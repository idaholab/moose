#ifndef QUADRPTDATA_H
#define QUADRPTDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"

//Forward Declarations
class MooseSystem;
class QGauss;
class DofMap;
class FEBase;
class Material;
template<class T> class NumericVector;
template<class T> class DenseVector;
template<class T> class DenseSubVector;
template<class T> class DenseMatrix;

/**
 * Holds quadrature point related data
 */
class QuadraturePointData
{
public:
  QuadraturePointData(MooseSystem & moose_system);
  virtual ~QuadraturePointData();

  void sizeEverything();

  void init();

  void reinit(THREAD_ID tid, unsigned int block_id, const NumericVector<Number>& soln, const Elem * elem);

  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * finite element.
   */
  std::vector<std::map<FEType, FEBase *> > _fe;

  /**
   * quadrature rule.
   */
  std::vector<QGauss *> _qrule;

  /**
   * number of quadrature points for current element
   */
  unsigned int _n_qpoints;
  
  /**
   * XYZ coordinates of quadrature points
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _q_point;

  /**
   * Side Jacobian pre-multiplied by the weight.
   */
  std::vector<std::map<FEType, const std::vector<Real> *> > _JxW;

  /**
   * Side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _phi;

  /**
   * Gradient of side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _grad_phi;

  /**
   * Second derivative of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _second_phi;


  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the quadrature points
   *
   * NOTE: This variable is used differently in ElementData and FaceData
   * In ElementData, the mapping uses only index zero (0), since all variables lives
   * inside the whole domain.
   * In FaceData, the mapping goes from boundary id to list of variables active
   * on this boundary (not all variables are needed on all boundaries)
   */
  std::map<unsigned int, std::vector<unsigned int> > _var_nums;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _var_grads;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _var_grads_older;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealTensor> > > _var_seconds;


  /**
   * Map to vector of auxiliary variable numbers that need to be evaluated
   * at the quadrature points
   */
  std::map<unsigned int, std::vector<unsigned int> > _aux_var_nums;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _aux_var_vals;

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _aux_var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _aux_var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _aux_var_grads;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _aux_var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _aux_var_grads_older;

  /**
   * Pointer to the material that is valid for the current block.
   */
  std::vector<Material *> _material;
};

#endif // QUADRPTDATA_H
