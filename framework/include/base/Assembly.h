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

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <vector>
#include "Moose.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
// libMesh
#include "dof_map.h"
#include "dense_matrix.h"
#include "dense_vector.h"
#include "coupling_matrix.h"
#include "fe.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"

// MOOSE Forward Declares
class MooseMesh;
class ArbitraryQuadrature;
class SystemBase;

/**
 * Keeps track of stuff related to assembling
 *
 */
class Assembly
{
public:
  Assembly(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid);
  virtual ~Assembly();

  /**
   * Get a reference to a pointer that will contain the current "volume" FE.
   * Note that when returned the pointer might initially be NULL.
   * It won't get an actual value until reinit is called.
   * @param type The type of FE
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFE(FEType type);

  /**
   * Get a reference to a pointer that will contain the current "face" FE.
   * Note that when returned the pointer might initially be NULL.
   * It won't get an actual value until reinit is called.
   * @param type The type of FE
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFEFace(FEType type);

  /**
   * Get a reference to a pointer that will contain the current "neighbor" FE.
   * Note that when returned the pointer might initially be NULL.
   * It won't get an actual value until reinit is called.
   * @param type The type of FE
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFEFaceNeighbor(FEType type);

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  QBase * & qRule() { return _current_qrule; }

  /**
   * Returns the reference to the quadrature points
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Point> & qPoints() { return _current_q_points; }

  /**
   * The current points in physical space where we have reinited through reinitAtPhysical()
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Point> & physicalPoints() { return _current_physical_points; }

  /**
   * Returns the reference to the transformed jacobian weights
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Real> & JxW() { return _current_JxW; }

  /**
   * Returns the reference to the coordinate transformation coefficients
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Real> & coordTransformation() { return _coord; }

  /**
   * Returns the reference to the current quadrature being used on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * & qRuleFace() { return _current_qrule_face; }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Point> & qPointsFace() { return _current_q_points_face; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Real> & JxWFace() { return _current_JxW_face; }

  /**
   * Returns the array of normals for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Point> & normals() { return _current_normals; }

  /**
   * Return the current element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * & elem() { return _current_elem; }

  /**
   * Returns the reference to the current element volume
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & elemVolume() { return _current_elem_volume; }

  /**
   * Returns the current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  unsigned int & side() { return _current_side; }

  /**
   * Returns the side element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * & sideElem() { return _current_side_elem; }

  /**
   * Returns the reference to the volume of current side element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & sideElemVolume() { return _current_side_volume; }

  /**
   * Return the neighbor element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * & neighbor() { return _current_neighbor_elem; }

  /**
   * Returns the reference to the node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node * & node() { return _current_node; }

  /**
   * Returns the reference to the neighboring node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node * & nodeNeighbor() { return _current_neighbor_node; }

  /**
   * Creates the volume, face and arbitrary qrules based on the Order passed in.
   */
  void createQRules(QuadratureType type, Order o);

  /**
   * Set the qrule to be used for volume integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   *
   * @param qrule The qrule you want to set
   * @param dim The spatial dimension of the qrule
   */
  void setVolumeQRule(QBase * qrule, unsigned int dim);

  /**
   * Set the qrule to be used for face integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   *
   * @param qrule The qrule you want to set
   * @param dim The spatial dimension of the qrule
   */
  void setFaceQRule(QBase * qrule, unsigned int dim);

  /**
   * Reinitialize objects (JxW, q_points, ...) for an elements
   *
   * @param elem The element we want to reinitialize on
   */
  void reinit(const Elem * elem);

  /**
   * Reinitialize the assembly data at specific physical point in the given element.
   */
  void reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points);

  /**
   * Reinitialize the assembly data at specific points in the reference element.
   */
  void reinit(const Elem * elem, const std::vector<Point> & reference_points);

  /**
   * Reinitialize the assembly data on an side of an element
   */
  void reinit(const Elem * elem, unsigned int side);

  /**
   * Reinitialize element and its neighbor
   * @param elem Element being reinitialized
   * @param side Side of the element
   * @param neighbor Neighbor facing the element on the side 'side'
   */
  void reinit(const Elem * elem, unsigned int side, const Elem * neighbor);

  /**
   * Reinitializes the neighbor's face at the physical coordinates given.
   */
  void reinitNeighborAtPhysical(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points);

  /**
   * Reinitialize assembly data for a node
   */
  void reinit(const Node * node);

  /**
   * Reinitialize assembly data for a neighbor node
   */
  void reinitNodeNeighbor(const Node * node);

  void init();

  void prepare();
  void prepareNeighbor();
  void prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices);
  void prepareScalar();
  void prepareOffDiagScalar();

  void copyShapes(unsigned int var);
  void copyFaceShapes(unsigned int var);
  void copyNeighborShapes(unsigned int var);

  void addResidual(NumericVector<Number> & residual);
  void addResidualNeighbor(NumericVector<Number> & residual);
  void addResidualScalar(NumericVector<Number> & residual);

  /**
   * Takes the values that are currently in _sub_Re and appends them to the cached values.
   */
  void cacheResidual();

  /**
   * Takes the values that are currently in _sub_Ke and appends them to the cached values.
   */
  void cacheResidualNeighbor();

  /**
   * Adds the values that have been cached by calling cacheResidual() and or cacheResidualNeighbor() to the residual.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidual(NumericVector<Number> & residual);

  void setResidual(NumericVector<Number> & residual);
  void setResidualNeighbor(NumericVector<Number> & residual);

  void addJacobian(SparseMatrix<Number> & jacobian);
  void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices);
  void addJacobianScalar(SparseMatrix<Number> & jacobian);

  /**
   * Takes the values that are currently in _sub_Kee and appends them to the cached values.
   */
  void cacheJacobian();

  /**
   * Takes the values that are currently in the neighbor Dense Matrices and appends them to the cached values.
   */
  void cacheJacobianNeighbor();

  /**
   * Adds the values that have been cached by calling cacheJacobian() and or cacheJacobianNeighbor() to the jacobian matrix.
   *
   * Note that this will also clear the cache.
   */
  void addCachedJacobian(SparseMatrix<Number> & jacobian);

  DenseVector<Number> & residualBlock(unsigned int var_num) { return _sub_Re[var_num]; }
  DenseVector<Number> & residualBlockNeighbor(unsigned int var_num) { return _sub_Rn[var_num]; }
  DenseVector<Number> & residualBlockScalar(unsigned int var_num) { return _scalar_Re[var_num]; }

  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar) { return _sub_Kee[ivar][jvar]; }
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar);
  DenseMatrix<Number> & jacobianBlockScalar(unsigned int ivar, unsigned int jvar) { return _scalar_Kee[ivar][jvar]; }
  DenseMatrix<Number> & jacobianBlockScalarLM(unsigned int ivar, unsigned int jvar) { return _scalar_Ken[ivar][jvar]; }
  DenseMatrix<Number> & jacobianBlockScalarCED(unsigned int ivar, unsigned int jvar) { return _scalar_Kne[ivar][jvar]; }
  void cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<unsigned int> & idof_indices, std::vector<unsigned int> & jdof_indices, Real scaling_factor);

  std::vector<std::pair<unsigned int, unsigned int> > & couplingEntries() { return _cm_entry; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  const std::vector<std::vector<RealTensor> > & secondPhi() { return _second_phi; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  const std::vector<std::vector<RealTensor> > & secondPhiFace() { return _second_phi_face; }

  const std::vector<std::vector<Real> > & phiFaceNeighbor() { return _phi_face_neighbor; }
  const std::vector<std::vector<RealGradient> > & gradPhiFaceNeighbor() { return _grad_phi_face_neighbor; }
  const std::vector<std::vector<RealTensor> > & secondPhiFaceNeighbor() { return _second_phi_face_neighbor; }


  const std::vector<std::vector<Real> > & fePhi(FEType type) { return _fe_phi[type]; }
  const std::vector<std::vector<RealGradient> > & feGradPhi(FEType type) { return _fe_grad_phi[type]; }
  const std::vector<std::vector<RealTensor> > & feSecondPhi(FEType type) { _need_second_derivative[type] = true; return _fe_second_phi[type]; }

  const std::vector<std::vector<Real> > & fePhiFace(FEType type) { return _fe_phi_face[type]; }
  const std::vector<std::vector<RealGradient> > & feGradPhiFace(FEType type) { return _fe_grad_phi_face[type]; }
  const std::vector<std::vector<RealTensor> > & feSecondPhiFace(FEType type) { _need_second_derivative[type] = true; return _fe_second_phi_face[type]; }

  const std::vector<std::vector<Real> > & fePhiFaceNeighbor(FEType type) { return _fe_phi_face_neighbor[type]; }
  const std::vector<std::vector<RealGradient> > & feGradPhiFaceNeighbor(FEType type) { return _fe_grad_phi_face_neighbor[type]; }
  const std::vector<std::vector<RealTensor> > & feSecondPhiFaceNeighbor(FEType type) { _need_second_derivative[type] = true; return _fe_second_phi_face_neighbor[type]; }

  std::map<FEType, bool> _need_second_derivative;

protected:
  /**
   * Just an internal helper function to reinit the volume FE objects.
   *
   * @param elem The element we are using to reinit
   */
  void reinitFE(const Elem * elem);

  /**
   * Just an internal helper function to reinit the face FE objects.
   *
   * @param elem The element we are using to reinit
   * @param side The side of the element we are reiniting on
   */
  void reinitFEFace(const Elem * elem, unsigned int side);

  void addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, const std::vector<unsigned int> & dof_indices, Real scaling_factor);
  void cacheResidualBlock(DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, const std::vector<unsigned int> & idof_indices, const std::vector<unsigned int> & jdof_indices, Real scaling_factor);

  SystemBase & _sys;
  /// Reference to coupling matrix
  CouplingMatrix * & _cm;
  /// Entries in the coupling matrix
  std::vector<std::pair<unsigned int, unsigned int> > _cm_entry;
  /// DOF map
  const DofMap & _dof_map;
  /// Thread number (id)
  THREAD_ID _tid;

  MooseMesh & _mesh;

  unsigned int _mesh_dimension;

  /// The "volume" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe;
  /// The "face" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe_face;
  /// The "neighbor" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe_neighbor;

  /**** Volume Stuff ****/

  /// Each dimension's actual fe objects indexed on type
  std::map<unsigned int, std::map<FEType, FEBase *> > _fe;
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_helper;
  /// The current helper object for transforming coordinates
  FEBase * _current_fe_helper;
  /// The current current quadrature rule being used (could be either volumetric or arbitrary - for dirac kernels)
  QBase * _current_qrule;
  /// The current volumetric quadrature for the element
  QBase * _current_qrule_volume;
  /// The current arbitrary quadrature rule used within the element interior
  ArbitraryQuadrature * _current_qrule_arbitrary;
  /// The current list of quadrature points
  std::vector<Point> _current_q_points;
  /// The current list of transformed jacobian weights
  std::vector<Real> _current_JxW;
  /// The current coordinate transformation coefficients
  std::vector<Real> _coord;
  /// Holds volume qrules for each dimension
  std::map<unsigned int, QBase *> _holder_qrule_volume;
  /// Holds arbitrary qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_arbitrary;
  /// Holds pointers to the dimension's q_points
  std::map<unsigned int, const std::vector<Point> *> _holder_q_points;
  /// Holds pointers to the dimension's transformed jacobian weights
  std::map<unsigned int, const std::vector<Real> *> _holder_JxW;

  /**** Face Stuff ****/

  /// types of finite elements
  std::map<unsigned int, std::map<FEType, FEBase *> > _fe_face;
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_face_helper;
  /// helper object for transforming coordinates
  FEBase * _current_fe_face_helper;
  /// quadrature rule used on faces
  QBase * _current_qrule_face;
  /// The current arbitrary quadrature rule used on element faces
  ArbitraryQuadrature * _current_qface_arbitrary;
  /// The current quadrature points on a face
  std::vector<Point> _current_q_points_face;
  /// The current transformed jacobian weights on a face
  std::vector<Real> _current_JxW_face;
  /// The current Normal vectors at the quadrature points.
  std::vector<Point> _current_normals;
  /// Holds face qrules for each dimension
  std::map<unsigned int, QBase *> _holder_qrule_face;
  /// Holds arbitrary face qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qface_arbitrary;
  /// Holds pointers to the dimension's q_points on a face
  std::map<unsigned int, const std::vector<Point> *> _holder_q_points_face;
  /// Holds pointers to the dimension's transformed jacobian weights on a face
  std::map<unsigned int, const std::vector<Real> *> _holder_JxW_face;
  /// Holds pointers to the dimension's normal vectors
  std::map<unsigned int, const std::vector<Point> *> _holder_normals;

  /**** Neighbor Stuff ****/

  /// types of finite elements
  std::map<unsigned int, std::map<FEType, FEBase *> > _fe_neighbor;


  /// The current "element" we are currently on.
  const Elem * _current_elem;
  /// Volume of the current element
  Real _current_elem_volume;
  /// The current side of the selected element (valid only when working with sides)
  unsigned int _current_side;
  /// The current "element" making up the side we are currently on.
  const Elem * _current_side_elem;
  /// Volume of the current side element
  Real _current_side_volume;
  /// The current neighbor "element"
  const Elem * _current_neighbor_elem;
  /// The current node we are working with
  const Node * _current_node;
  /// The current neighboring node we are working with
  const Node * _current_neighbor_node;

  /// This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
  std::vector<Point> _current_physical_points;

  /// residual contributions for each variable from the element
  std::vector<DenseVector<Number> > _sub_Re;
  /// residual contributions for each variable from the neighbor
  std::vector<DenseVector<Number> > _sub_Rn;
  /// residual for scalar variable
  std::vector<DenseVector<Number> > _scalar_Re;
  /// auxiliary vector for scaling residuals (optimization to avoid expensive construction/destruction)
  DenseVector<Number> _tmp_Re;

  /// jacobian contributions
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kee;

  /// jacobian contributions from the element and neighbor
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Ken;
  /// jacobian contributions from the neighbor and element
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kne;
  /// jacobian contributions from the neighbor
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Knn;

  /// jacobian contributions for scalar variable
  std::vector<std::vector<DenseMatrix<Number> > > _scalar_Kee;
  /// jacobian contributions
  std::vector<std::vector<DenseMatrix<Number> > > _scalar_Ken;
  /// jacobian contributions
  std::vector<std::vector<DenseMatrix<Number> > > _scalar_Kne;
  bool _scalar_has_off_diag_contributions;

  /// auxiliary matrix for scaling jacobians (optimization to avoid expensive construction/destruction)
  DenseMatrix<Number> _tmp_Ke;

  // Shape function values, gradients. second derivatives
  std::vector<std::vector<Real> > _phi;
  std::vector<std::vector<RealGradient> > _grad_phi;
  std::vector<std::vector<RealTensor> > _second_phi;

  std::vector<std::vector<Real> > _phi_face;
  std::vector<std::vector<RealGradient> > _grad_phi_face;
  std::vector<std::vector<RealTensor> > _second_phi_face;

  std::vector<std::vector<Real> > _phi_face_neighbor;
  std::vector<std::vector<RealGradient> > _grad_phi_face_neighbor;
  std::vector<std::vector<RealTensor> > _second_phi_face_neighbor;

  // Shape function values, gradients. second derivatives for each FE type
  std::map<FEType, std::vector<std::vector<Real> > > _fe_phi;
  std::map<FEType, std::vector<std::vector<RealGradient> > > _fe_grad_phi;
  std::map<FEType, std::vector<std::vector<RealTensor> > > _fe_second_phi;

  std::map<FEType, std::vector<std::vector<Real> > > _fe_phi_face;
  std::map<FEType, std::vector<std::vector<RealGradient> > > _fe_grad_phi_face;
  std::map<FEType, std::vector<std::vector<RealTensor> > > _fe_second_phi_face;

  std::map<FEType, std::vector<std::vector<Real> > > _fe_phi_face_neighbor;
  std::map<FEType, std::vector<std::vector<RealGradient> > > _fe_grad_phi_face_neighbor;
  std::map<FEType, std::vector<std::vector<RealTensor> > > _fe_second_phi_face_neighbor;


  /// Values cached by calling cacheResidual()
  std::vector<Real> _cached_residual_values;
  /// Where the cached values should go
  std::vector<unsigned int> _cached_residual_rows;

  unsigned int _max_cached_residuals;

  /// Values cached by calling cacheJacobian()
  std::vector<Real> _cached_jacobian_values;
  /// Row where the corresponding cached value should go
  std::vector<unsigned int> _cached_jacobian_rows;
  /// Column where the corresponding cached value should go
  std::vector<unsigned int> _cached_jacobian_cols;

  unsigned int _max_cached_jacobians;
};

#endif /* ASSEMBLY_H */
