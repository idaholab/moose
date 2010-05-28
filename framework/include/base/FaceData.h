#ifndef FACEDATA_H
#define FACEDATA_H

#include "Moose.h"
#include "MooseArray.h"

//libMesh includes
#include "transient_system.h"

//Forward Declarations
class MooseSystem;
class QGauss;
class FEBase;

template <class T> class NumericVector;

class FaceData
{
public:
  FaceData(MooseSystem & moose_system);
  virtual ~FaceData();

  void sizeEverything();

  void init();

  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id);
  void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual);

public:
  /**
   * The MooseSystem this Kernel is associated with.
    */
  MooseSystem & _moose_system;

  /// BCs
  /**
   * Current node for nodal BC's
   */
  std::vector<const Node *> _current_node;

  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  std::vector<NumericVector<Number> *> _current_residual;

  /**
   * Current side.
   */
  std::vector<unsigned int> _current_side;

  /**
   * Boundary finite element.
   */
  std::vector<std::map<FEType, FEBase *> > _fe_face;

  /**
   * Boundary quadrature rule.
   */
  std::vector<QGauss *> _qface;

  /**
   * XYZ coordinates of quadrature points
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _q_point_face;

  /**
   * Side Jacobian pre-multiplied by the weight.
   */
  std::vector<std::map<FEType, const std::vector<Real> *> > _JxW_face;

  /**
   * Side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _phi_face;

  /**
   * Gradient of side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _dphi_face;

  /**
   * Second derivative of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _d2phi_face;

  /**
   * Normal vectors at the quadrature points.
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _normals_face;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the quadrature points on that boundary
   */
  std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the nodes on that boundary
   */
  std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums_nodal;

  /**
   * Holds the current dof numbers for each variable for nodal bcs
   */
  std::vector<std::vector<unsigned int> > _nodal_bc_var_dofs;

  /**
   * ***************
   * Values of stuff
   * ***************
   */

  /**
   * Value of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals_face;

  /**
   * Gradient of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealGradient> > > _var_grads_face;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  MooseArray<MooseArray<MooseArray<RealTensor> > > _var_seconds_face;

  /**
   * Value of the variables at the nodes.
   */
  MooseArray<MooseArray<MooseArray<Real> > > _var_vals_face_nodal;
};

  

#endif //FACEDATA_H
