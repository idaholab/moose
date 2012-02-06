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

  FEBase * & getFE(FEType type);

  /**
   * Returns the reference to the current quadrature being used
   */
  QBase * & qRule() { return _qrule; }

  /**
   * Returns the reference to the quadrature points
   */
  const std::vector<Point> & qPoints() { return _q_points; }

  /**
   * TODO:
   */
  const std::vector<Point> & physicalPoints() { return _current_physical_points; }

  /**
   * Returns the reference to the transformed jacobian weights
   */
  const std::vector<Real> & JxW() { return _JxW; }

  /**
   * Returns the reference to the coordinate transformation coefficients
   */
  const std::vector<Real> & coordTransformation() { return _coord; }

  FEBase * & getFEFace(FEType type);

  /**
   * Returns the reference to the current quadrature being used on a current face
   */
  QBase * & qRuleFace() { return _qrule_face; }

  /**
   * Returns the reference to the current quadrature being used
   */
  const std::vector<Point> & qPointsFace() { return _q_points_face; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   */
  const std::vector<Real> & JxWFace() { return _JxW_face; }

  FEBase * & getFEFaceNeighbor(FEType type);

  /**
   * Returns the array of normals for quadrature points on a current side
   */
  const std::vector<Point> & normals() { return _normals; }

  /**
   * Return the current element
   */
  const Elem * & elem() { return _current_elem; }

  /**
   * Returns the reference to the current element volume
   */
  const Real & elemVolume() { return _current_elem_volume; }

  /**
   * Returns the current side
   */
  unsigned int & side() { return _current_side; }

  /**
   * Returns the side element
   */
  const Elem * & sideElem() { return _current_side_elem; }

  /**
   * Returns the reference to the volume of current side element
   */
  const Real & sideElemVolume() { return _current_side_volume; }

  /**
   * Return the neighbor element
   */
  const Elem * & neighbor() { return _neighbor_elem; }


  /**
   * Returns the reference to the node
   */
  const Node * & node() { return _current_node; }

  /**
   * Returns the reference to the neighboring node
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
   */
  void setVolumeQRule(QBase * qrule);

  /**
   * Set the qrule to be used for face integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   */
  void setFaceQRule(QBase * qrule);

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

  ///

  void init();

  void prepare();
  void prepareNeighbor();
  void prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<unsigned int> & dof_indices);
  void prepareScalar(MooseVariableScalar & var, MooseVariable & ced_var);

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
  DenseVector<Number> & residualBlockScalar() { return _scalar_Re; }

  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar) { return _sub_Kee[ivar][jvar]; }
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar);
  DenseMatrix<Number> & jacobianBlockScalar() { return _scalar_Kee; }
  // FIXME: UGLY!!!
  DenseMatrix<Number> & jacobianBlockScalarOffdiag(int type) { return (type == 0) ? _scalar_Ken : _scalar_Kne; }
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

protected:
  void addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, const std::vector<unsigned int> & dof_indices, Real scaling_factor);
  void cacheResidualBlock(DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<unsigned int> & dof_indices, Real scaling_factor);

  void addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, const std::vector<unsigned int> & idof_indices, const std::vector<unsigned int> & jdof_indices, Real scaling_factor);

  SystemBase & _sys;
  CouplingMatrix * & _cm;                                        ///< Reference to coupling matrix
  std::vector<std::pair<unsigned int, unsigned int> > _cm_entry; ///< Entries in the coupling matrix
  const DofMap & _dof_map;                                       ///< DOF map
  THREAD_ID _tid;                                                ///< Thread number (id)

  MooseMesh & _mesh;

  std::map<FEType, FEBase *> _fe;               ///< types of finite elements
  FEBase * _fe_helper;                          ///< helper object for transforming coordinates
  QBase * _qrule;                               ///< current quadrature rule being used (could be either volumetric or arbitrary - for dirac kernels)
  QBase * _qrule_volume;                        ///< volumetric quadrature for the element
  ArbitraryQuadrature * _qrule_arbitrary;       ///< arbitrary quadrature rule used within the element interior
  ArbitraryQuadrature * _qface_arbitrary;       ///< arbitrary quadrature rule used on element faces

  const std::vector<Point> & _q_points;         ///< reference to the list of quadrature points
  const std::vector<Real> & _JxW;               ///< reference to the list of transformed jacobian weights
  std::vector<Real> _coord;                     ///< reference to the coordinate transformation coefficients

  std::map<FEType, FEBase *> _fe_face;          ///< types of finite elements
  FEBase * _fe_face_helper;                     ///< helper object for transforming coordinates
  QBase * _qrule_face;                          ///< quadrature rule used on faces
  const std::vector<Point> & _q_points_face;    ///< reference to the quadrature points on a face
  const std::vector<Real> & _JxW_face;          ///< reference to the transformed jacobian weights on a face
  const std::vector<Point> & _normals;          ///< Normal vectors at the quadrature points.

  std::map<FEType, FEBase *> _fe_neighbor;      ///< types of finite elements
  FEBase * _fe_neighbor_helper;                 ///< helper object for transforming coordinates

  const Elem * _current_elem;                   ///< The current "element" we are currently on.
  Real _current_elem_volume;                    ///< Volume of the current element
  unsigned int _current_side;                   ///< The current side of the selected element (valid only when working with sides)
  const Elem * _current_side_elem;              ///< The current "element" making up the side we are currently on.
  Real _current_side_volume;                    ///< Volume of the current side element
  const Elem * _neighbor_elem;                  ///< The current neighbor "element"

  const Node * _current_node;                   ///< The current node we are working with
  const Node * _current_neighbor_node;          ///< The current neighboring node we are working with

  std::vector<Point> _current_physical_points;  ///< This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.

  MooseVariableScalar * _scalar_var;                 ///< scalar variable number
  MooseVariable * _ced_var;                          ///< constrained variable number

  std::vector<DenseVector<Number> > _sub_Re;                     ///< residual contributions for each variable from the element
  std::vector<DenseVector<Number> > _sub_Rn;                     ///< residual contributions for each variable from the neighbor
  DenseVector<Number> _scalar_Re;                                ///< residual for scalar variable
  DenseVector<Number> _tmp_Re;                                   ///< auxiliary vector for scaling residuals (optimization to avoid expensive construction/destruction)

  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kee;      ///< jacobian contributions

  std::vector<std::vector<DenseMatrix<Number> > > _sub_Ken;      ///< jacobian contributions from the element and neighbor
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Kne;      ///< jacobian contributions from the neighbor and element
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Knn;      ///< jacobian contributions from the neighbor

  DenseMatrix<Number> _scalar_Kee;   ///< jacobian contributions for scalar variable
  DenseMatrix<Number> _scalar_Ken;   ///< jacobian contributions
  DenseMatrix<Number> _scalar_Kne;   ///< jacobian contributions

  DenseMatrix<Number> _tmp_Ke;                                   ///< auxiliary matrix for scaling jacobians (optimization to avoid expensive construction/destruction)

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

  std::vector<Real> _cached_residual_values;                     /// Values cached by calling cacheResidual()
  std::vector<unsigned int> _cached_residual_rows;               /// Where the cached values should go

  unsigned int _max_cached_residuals;

  std::vector<Real> _cached_jacobian_values;                     /// Values cached by calling cacheJacobian()
  std::vector<unsigned int> _cached_jacobian_rows;               /// Row where the corresponding cached value should go
  std::vector<unsigned int> _cached_jacobian_cols;               /// Column where the corresponding cached value should go

  unsigned int _max_cached_jacobians;
};

#endif /* ASSEMBLY_H */
