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
template<class T> class NumericVector;
template<class T> class DenseVector;
template<class T> class DenseSubVector;
template<class T> class DenseMatrix;

/**
 * Holds quadrature point related data
 */
class QuadrPtData
{
public:
  QuadrPtData(MooseSystem & moose_system);
  virtual ~QuadrPtData();

  void sizeEverything();

  void init();

  /**
   * The MooseSystem this Kernel is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Boundary finite element.
   */
  std::vector<std::map<FEType, FEBase *> > _fe;

  /**
   * Boundary quadrature rule.
   */
  std::vector<QGauss *> _qrule;

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
  std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _dphi;

  /**
   * Second derivative of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _d2phi;
};

#endif // QUADRPTDATA_H
