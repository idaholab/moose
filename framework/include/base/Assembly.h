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
#include "ParallelUniqueId.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "MooseTypes.h"
// libMesh
#include "libmesh/dof_map.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/coupling_matrix.h"
#include "libmesh/fe.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

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
   * Build FEs with a type
   * @param type The type of FE
   */
  void buildFE(FEType type);

  /**
   * Build FEs for a face with a type
   * @param type The type of FE
   */
  void buildFaceFE(FEType type);

  /**
   * Build FEs for a neighbor face with a type
   * @param type The type of FE
   */
  void buildFaceNeighborFE(FEType type);

   /**
   * Get a reference to a pointer that will contain the current volume FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFE(FEType type, unsigned int dim);

  /**
   * Get a reference to a pointer that will contain the current "face" FE.
   * @param type The type of FE
   * @param dim The dimension of the current face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFEFace(FEType type, unsigned int dim);

  /**
   * Get a reference to a pointer that will contain the current "neighbor" FE.
   * @param type The type of FE
   * @param dim The dimension of the neighbor face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase * & getFEFaceNeighbor(FEType type, unsigned int dim);

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  QBase * & qRule() { return _current_qrule; }

  /**
   * Returns the reference to the quadrature points
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & qPoints() { return _current_q_points; }

  /**
   * The current points in physical space where we have reinited through reinitAtPhysical()
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & physicalPoints() { return _current_physical_points; }

  /**
   * Returns the reference to the transformed jacobian weights
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxW() { return _current_JxW; }

  /**
   * Returns the reference to the coordinate transformation coefficients
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & coordTransformation() { return _coord; }

  /**
   * Get the coordinate system type
   * @return A reference to the coordinate system type
   */
  const Moose::CoordinateSystemType & coordSystem() { return _coord_type; }

  /**
   * Returns the reference to the current quadrature being used on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * & qRuleFace() { return _current_qrule_face; }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & qPointsFace() { return _current_q_points_face; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWFace() { return _current_JxW_face; }

  /**
   * Returns the array of normals for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & normals() { return _current_normals; }

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
   * Returns the current neighboring side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  unsigned int & neighborSide() { return _current_neighbor_side; }

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
   * Returns the reference to the current neighbor volume
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & neighborVolume() { return _current_neighbor_volume; }

  /**
   * Returns the reference to the current quadrature being used on a current neighbor
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * & qRuleNeighbor() { return _current_qrule_neighbor; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWNeighbor() { return _current_JxW_neighbor; }

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
   * Creates the volume, face and arbitrary qrules based on the orders passed in.
   */
  void createQRules(QuadratureType type, Order order, Order volume_order, Order face_order);

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
   * Set the qrule to be used for neighbor integration.
   *
   * Note: This is normally set internally, only use if you know what you are doing!
   *
   * @param qrule The qrule you want to set
   * @param dim The spatial dimension of the qrule
   */
  void setNeighborQRule(QBase * qrule, unsigned int dim);

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
   * Reinitialize an element and its neighbor along a particular side.
   *
   * @param elem Element being reinitialized
   * @param side Side of the element
   * @param neighbor Neighbor facing the element on the side 'side'
   * @param neighbor_side The side id on the neighboring element.
   */
  void reinitElemAndNeighbor(const Elem * elem, unsigned int side, const Elem * neighbor, unsigned int neighbor_side);

  /**
   * Reinitializes the neighbor at the reference coordinates given.
   */
  void reinitNeighborAtReference(const Elem * neighbor, const std::vector<Point> & reference_points);

  /**
   * Reinitializes the neighbor at the physical coordinates given.
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

  /**
   * Whether or not this assembly should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  void useFECache(bool fe_cache) { _should_use_fe_cache = fe_cache; }

  void prepare();

  /**
   * Used for preparing the dense residual and jacobian blocks for one particular variable.
   *
   * @param var The variable that needs to have it's datastructures prepared
   */
  void prepareVariable(MooseVariable * var);
  void prepareNeighbor();
  void prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<dof_id_type> & dof_indices);
  void prepareScalar();
  void prepareOffDiagScalar();

  void copyShapes(unsigned int var);
  void copyFaceShapes(unsigned int var);
  void copyNeighborShapes(unsigned int var);

  void addResidual(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_NONTIME);
  void addResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_NONTIME);
  void addResidualScalar(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_NONTIME);

  /**
   * Takes the values that are currently in _sub_Re and appends them to the cached values.
   */
  void cacheResidual();

  /**
   * Cache individual residual contributions.  These will ultimately get added to the residual when addCachedResidual() is called.
   *
   * @param dof The degree of freedom to add the residual contribution to
   * @param value The value of the residual contribution.
   * @param type Whether the contribution should go to the Time or Non-Time residual
   */
  void cacheResidualContribution(dof_id_type dof, Real value, Moose::KernelType type);

  /**
   * Takes the values that are currently in _sub_Ke and appends them to the cached values.
   */
  void cacheResidualNeighbor();

  /**
   * Adds the values that have been cached by calling cacheResidual() and or cacheResidualNeighbor() to the residual.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidual(NumericVector<Number> & residual, Moose::KernelType type);

  void setResidual(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_NONTIME);
  void setResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_NONTIME);

  void addJacobian(SparseMatrix<Number> & jacobian);
  void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian);
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, std::vector<dof_id_type> & neighbor_dof_indices);
  void addJacobianScalar(SparseMatrix<Number> & jacobian);
  void addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar);

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

  DenseVector<Number> & residualBlock(unsigned int var_num, Moose::KernelType type = Moose::KT_NONTIME) { return _sub_Re[static_cast<unsigned int>(type)][var_num]; }
  DenseVector<Number> & residualBlockNeighbor(unsigned int var_num, Moose::KernelType type = Moose::KT_NONTIME) { return _sub_Rn[static_cast<unsigned int>(type)][var_num]; }

  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar);
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar);
  void cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<dof_id_type> & idof_indices, std::vector<dof_id_type> & jdof_indices, Real scaling_factor);

  std::vector<std::pair<MooseVariable *, MooseVariable *> > & couplingEntries() { return _cm_entry; }

  const VariablePhiValue & phi() { return _phi; }
  const VariablePhiGradient & gradPhi() { return _grad_phi; }
  const VariablePhiSecond & secondPhi() { return _second_phi; }

  const VariablePhiValue & phiFace() { return _phi_face; }
  const VariablePhiGradient & gradPhiFace() { return _grad_phi_face; }
  const VariablePhiSecond & secondPhiFace() { return _second_phi_face; }

  const VariablePhiValue & phiFaceNeighbor() { return _phi_face_neighbor; }
  const VariablePhiGradient & gradPhiFaceNeighbor() { return _grad_phi_face_neighbor; }
  const VariablePhiSecond & secondPhiFaceNeighbor() { return _second_phi_face_neighbor; }


  const VariablePhiValue & fePhi(FEType type);
  const VariablePhiGradient & feGradPhi(FEType type);
  const VariablePhiSecond & feSecondPhi(FEType type);

  const VariablePhiValue & fePhiFace(FEType type);
  const VariablePhiGradient & feGradPhiFace(FEType type);
  const VariablePhiSecond & feSecondPhiFace(FEType type);

  const VariablePhiValue & fePhiFaceNeighbor(FEType type);
  const VariablePhiGradient & feGradPhiFaceNeighbor(FEType type);
  const VariablePhiSecond & feSecondPhiFaceNeighbor(FEType type);

  /**
   * Invalidate any currently cached data.  In particular this will cause FE data to get recached.
   */
  void invalidateCache();

  std::map<FEType, bool> _need_second_derivative;

  /**
   * Caches the NodalBC Jacobian entry 'value', to eventually be
   * stored in the (i,j) location of the matrix.
   *
   * We can't add NodalBC values to the Jacobian (or preconditioning)
   * matrix at the time they are computed -- we instead need to
   * overwrite an entire row of values with the Jacobian associated to
   * the NodalBC, which may be coupled to other variables, *after*
   * matrix assembly is finished.
   *
   * We use numeric_index_type for the index arrays (rather than
   * dof_id_type) since that is what the SparseMatrix interface uses,
   * but at the time of this writing, those two types are equivalent.
   */
  void cacheNodalBCJacobianEntry(numeric_index_type i, numeric_index_type j, Real value);

  /**
   * Clears any cached NodalBC Jacobian entries that have been
   * accumulated during previous Assembly calls.
   */
  void clearCachedNodalBCJacobianEntries();

  /**
   * Sets previously-cached NodalBC Jacobian values via SparseMatrix::set() calls.
   */
  void setCachedNodalBCJacobianEntries(SparseMatrix<Number> & jacobian);

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

  void addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, const std::vector<dof_id_type> & dof_indices, Real scaling_factor);
  void cacheResidualBlock(std::vector<Real> & cached_residual_values,
                          std::vector<dof_id_type> & cached_residual_rows,
                          DenseVector<Number> & res_block,
                          std::vector<dof_id_type> & dof_indices,
                          Real scaling_factor);

  void setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<dof_id_type> & dof_indices, Real scaling_factor);

  void addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices, Real scaling_factor);

  SystemBase & _sys;
  /// Reference to coupling matrix
  CouplingMatrix * & _cm;
  /// Entries in the coupling matrix (only for field variables)
  std::vector<std::pair<MooseVariable *, MooseVariable *> > _cm_entry;
  /// Flag that indicates if the jacobian block was used
  std::vector<std::vector<unsigned char> > _jacobian_block_used;
  /// Flag that indicates if the jacobian block for neighbor was used
  std::vector<std::vector<unsigned char> > _jacobian_block_neighbor_used;
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
  MooseArray<Point> _current_q_points;
  /// The current list of transformed jacobian weights
  MooseArray<Real> _current_JxW;
  /// The coordinate system
  Moose::CoordinateSystemType _coord_type;
  /// The current coordinate transformation coefficients
  MooseArray<Real> _coord;
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
  MooseArray<Point> _current_q_points_face;
  /// The current transformed jacobian weights on a face
  MooseArray<Real> _current_JxW_face;
  /// The current Normal vectors at the quadrature points.
  MooseArray<Point> _current_normals;
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
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_neighbor_helper;

  /// quadrature rule used on neighbors
  QBase * _current_qrule_neighbor;
  /// Holds arbitrary qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_neighbor;
  /// The current transformed jacobian weights on a neighbor's face
  MooseArray<Real> _current_JxW_neighbor;

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
  /// The current side of the selected neighboring element (valid only when working with sides)
  unsigned int _current_neighbor_side;
  /// The current side element of the ncurrent neighbor element
  const Elem * _current_neighbor_side_elem;
  /// Volume of the current neighbor
  Real _current_neighbor_volume;
  /// The current node we are working with
  const Node * _current_node;
  /// The current neighboring node we are working with
  const Node * _current_neighbor_node;

  /// This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
  MooseArray<Point> _current_physical_points;

  /// residual contributions for each variable from the element
  std::vector<std::vector<DenseVector<Number> > > _sub_Re;
  /// residual contributions for each variable from the neighbor
  std::vector<std::vector<DenseVector<Number> > > _sub_Rn;
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

  /// auxiliary matrix for scaling jacobians (optimization to avoid expensive construction/destruction)
  DenseMatrix<Number> _tmp_Ke;

  // Shape function values, gradients. second derivatives
  VariablePhiValue _phi;
  VariablePhiGradient _grad_phi;
  VariablePhiSecond _second_phi;

  VariablePhiValue _phi_face;
  VariablePhiGradient _grad_phi_face;
  VariablePhiSecond _second_phi_face;

  VariablePhiValue _phi_face_neighbor;
  VariablePhiGradient _grad_phi_face_neighbor;
  VariablePhiSecond _second_phi_face_neighbor;

  class FEShapeData
  {
  public:
    MooseArray<std::vector<Real> > _phi;
    MooseArray<std::vector<RealGradient> > _grad_phi;
    MooseArray<std::vector<RealTensor> > _second_phi;
  };

  /**
   * Ok - here's the design.  One ElementFEShapeData class will be stored per element in _fe_shape_data_cache.
   * When reinit() is called on an element we will retrieve the ElementFEShapeData class associated with that
   * element.  If it's NULL we'll make one.  Then we'll store a copy of the shape functions computed on that
   * element within shape_data and JxW and q_points within EleementFEShapeData.
   */
  class ElementFEShapeData
  {
  public:
    /// This is where the cached shape functions will be held
    std::map<FEType, FEShapeData *> _shape_data;

    /// Whether or not this data is invalid (needs to be recached) note that there is no constructor so the value is invalid the first time through and must be set.
    bool _invalidated;

    /// Cached JxW
    MooseArray<Real> _JxW;

    /// Cached xyz positions of quadrature points
    MooseArray<Point> _q_points;
  };

  /// Cached shape function values stored by element
  std::map<dof_id_type, ElementFEShapeData * > _element_fe_shape_data_cache;

  /// Whether or not fe cache should be built at all
  bool _should_use_fe_cache;

  /// Whether or not fe should currently be cached - This will be false if something funky is going on with the quadrature rules.
  bool _currently_fe_caching;

  // Shape function values, gradients. second derivatives for each FE type
  std::map<FEType, FEShapeData * > _fe_shape_data;
  std::map<FEType, FEShapeData * > _fe_shape_data_face;
  std::map<FEType, FEShapeData * > _fe_shape_data_face_neighbor;

  /// Values cached by calling cacheResidual() (the first vector is for TIME vs NONTIME)
  std::vector<std::vector<Real> > _cached_residual_values;

  /// Where the cached values should go (the first vector is for TIME vs NONTIME)
  std::vector<std::vector<dof_id_type> > _cached_residual_rows;

  unsigned int _max_cached_residuals;

  /// Values cached by calling cacheJacobian()
  std::vector<Real> _cached_jacobian_values;
  /// Row where the corresponding cached value should go
  std::vector<dof_id_type> _cached_jacobian_rows;
  /// Column where the corresponding cached value should go
  std::vector<dof_id_type> _cached_jacobian_cols;

  unsigned int _max_cached_jacobians;

  /// Will be true if our preconditioning matrix is a block-diagonal matrix.  Which means that we can take some shortcuts.
  unsigned int _block_diagonal_matrix;

  /// Temporary work vector to keep from reallocating it
  std::vector<dof_id_type> _temp_dof_indices;

  /**
   * Storage for cached NodalBC data.
   */
  std::vector<Real> _cached_nodal_bc_vals;
  std::vector<numeric_index_type> _cached_nodal_bc_rows;
  std::vector<numeric_index_type> _cached_nodal_bc_cols;
};

#endif /* ASSEMBLY_H */
