//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseArray.h"
#include "MooseTypes.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/point.h"

#include "DualRealOps.h"

// libMesh forward declarations
namespace libMesh
{
class DofMap;
class CouplingMatrix;
class Elem;
template <typename>
class VectorValue;
typedef VectorValue<Real> RealVectorValue;
template <typename T>
class FEGenericBase;
typedef FEGenericBase<Real> FEBase;
typedef FEGenericBase<VectorValue<Real>> FEVectorBase;
class Node;
template <typename T>
class NumericVector;
template <typename T>
class SparseMatrix;
}

// MOOSE Forward Declares
class MooseMesh;
class ArbitraryQuadrature;
class SystemBase;
class MooseVariableFEBase;
class MooseVariableBase;
template <typename>
class MooseVariableFE;
class MooseVariableScalar;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;
class XFEMInterface;
class SubProblem;

/**
 * Keeps track of stuff related to assembling
 *
 */
class Assembly
{
public:
  Assembly(SystemBase & sys, THREAD_ID tid);
  virtual ~Assembly();

  /**
   * Get a reference to a pointer that will contain the current volume FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEBase * const & getFE(FEType type, unsigned int dim) const
  {
    buildFE(type);
    return _const_fe[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current 'neighbor' FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEBase * const & getFENeighbor(FEType type, unsigned int dim) const
  {
    buildNeighborFE(type);
    return _const_fe_neighbor[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current "face" FE.
   * @param type The type of FE
   * @param dim The dimension of the current face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEBase * const & getFEFace(FEType type, unsigned int dim) const
  {
    buildFaceFE(type);
    return _const_fe_face[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current "neighbor" FE.
   * @param type The type of FE
   * @param dim The dimension of the neighbor face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEBase * const & getFEFaceNeighbor(FEType type, unsigned int dim) const
  {
    buildFaceNeighborFE(type);
    return _const_fe_face_neighbor[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current volume FEVector.
   * @param type The type of FEVector
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEVectorBase * const & getVectorFE(FEType type, unsigned int dim) const
  {
    buildVectorFE(type);
    return _const_vector_fe[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current 'neighbor' FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEVectorBase * const & getVectorFENeighbor(FEType type, unsigned int dim) const
  {
    buildVectorNeighborFE(type);
    return _const_vector_fe_neighbor[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current "face" FE.
   * @param type The type of FE
   * @param dim The dimension of the current face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEVectorBase * const & getVectorFEFace(FEType type, unsigned int dim) const
  {
    buildVectorFaceFE(type);
    return _const_vector_fe_face[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current "neighbor" FE.
   * @param type The type of FE
   * @param dim The dimension of the neighbor face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEVectorBase * const & getVectorFEFaceNeighbor(FEType type, unsigned int dim) const
  {
    buildVectorFaceNeighborFE(type);
    return _const_vector_fe_face_neighbor[dim][type];
  }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const QBase * const & qRule() const { return _const_current_qrule; }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  QBase * const & writeableQRule() { return _current_qrule; }

  /**
   * Returns the reference to the quadrature points
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & qPoints() const { return _current_q_points; }

  /**
   * The current points in physical space where we have reinited through reinitAtPhysical()
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & physicalPoints() const { return _current_physical_points; }

  /**
   * Returns the reference to the transformed jacobian weights
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxW() const { return _current_JxW; }

  template <ComputeStage compute_stage>
  const MooseArray<ADReal> & adJxW() const
  {
    return _ad_JxW;
  }

  template <ComputeStage compute_stage>
  const MooseArray<ADReal> & adJxWFace() const
  {
    return _current_JxW_face;
  }

  template <ComputeStage compute_stage>
  const MooseArray<ADReal> & adCurvatures() const;

  /**
   * Returns the reference to the coordinate transformation coefficients
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & coordTransformation() const { return _coord; }

  /**
   * Returns the reference to the coordinate transformation coefficients on the mortar segment mesh
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & mortarCoordTransformation() const { return _coord_msm; }

  /**
   * Returns the reference to the AD version of the coordinate transformation coefficients
   * @return A _reference_.  Make sure to store this as a reference!
   */
  template <ComputeStage compute_stage>
  const MooseArray<ADReal> & adCoordTransformation() const
  {
    return _coord;
  }

  /**
   * Get the coordinate system type
   * @return A reference to the coordinate system type
   */
  const Moose::CoordinateSystemType & coordSystem() { return _coord_type; }

  /**
   * Returns the reference to the current quadrature being used on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const QBase * const & qRuleFace() const { return _const_current_qrule_face; }

  /**
   * Returns the reference to the current quadrature being used on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * const & writeableQRuleFace() { return _current_qrule_face; }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & qPointsFace() const { return _current_q_points_face; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWFace() const { return _current_JxW_face; }

  /**
   * Returns the array of normals for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & normals() const { return _current_normals; }

  /***
   * Returns the array of normals for quadrature points on a current side
   */
  const std::vector<Eigen::Map<RealDIMValue>> & mappedNormals() const { return _mapped_normals; }

  /**
   * Returns the array of tangents for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<std::vector<Point>> & tangents() const { return _current_tangents; }

  template <ComputeStage compute_stage>
  const ADPoint & adNormals() const
  {
    return _current_normals;
  }

  template <ComputeStage compute_stage>
  const ADPoint & adQPoints() const
  {
    return _current_q_points;
  }

  template <ComputeStage compute_stage>
  const ADPoint & adQPointsFace() const
  {
    return _current_q_points_face;
  }

  /**
   * Return the current element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * const & elem() const { return _current_elem; }

  /**
   * Return the current subdomain ID
   */
  const SubdomainID & currentSubdomainID() const { return _current_subdomain_id; }

  /**
   * set the current subdomain ID
   */
  void setCurrentSubdomainID(SubdomainID i) { _current_subdomain_id = i; }

  /**
   * Returns the reference to the current element volume
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & elemVolume() { return _current_elem_volume; }

  /**
   * Returns the current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const unsigned int & side() const { return _current_side; }

  /**
   * Returns the current neighboring side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const unsigned int & neighborSide() const { return _current_neighbor_side; }

  /**
   * Returns the side element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem *& sideElem() { return _current_side_elem; }

  /**
   * Returns the reference to the volume of current side element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & sideElemVolume() { return _current_side_volume; }

  /**
   * Return the neighbor element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * const & neighbor() const { return _current_neighbor_elem; }

  /**
   * Return the lower dimensional element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * const & lowerDElem() const { return _current_lower_d_elem; }

  /**
   * Return the current subdomain ID
   */
  const SubdomainID & currentNeighborSubdomainID() const { return _current_neighbor_subdomain_id; }

  /**
   * set the current subdomain ID
   */
  void setCurrentNeighborSubdomainID(SubdomainID i) { _current_neighbor_subdomain_id = i; }

  /**
   * Returns the reference to the current neighbor volume
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Real & neighborVolume()
  {
    _need_neighbor_elem_volume = true;
    return _current_neighbor_volume;
  }

  /**
   * Returns the reference to the current quadrature being used on a current neighbor
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const QBase * const & qRuleNeighbor() const { return _const_current_qrule_neighbor; }

  /**
   * Returns the reference to the current quadrature being used on a current neighbor
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * const & writeableQRuleNeighbor() { return _current_qrule_neighbor; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWNeighbor() const { return _current_JxW_neighbor; }

  /**
   * Returns the reference to the current quadrature points being used on the neighbor face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & qPointsFaceNeighbor() const { return _current_q_points_face_neighbor; }

  /**
   * Returns the reference to the node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node * const & node() const { return _current_node; }

  /**
   * Returns the reference to the neighboring node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node * const & nodeNeighbor() const { return _current_neighbor_node; }

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
   * Reinitialize FE data for the given element on the given side, optionally
   * with a given set of reference points
   */
  void reinitElemFaceRef(const Elem * elem,
                         unsigned int elem_side,
                         Real tolerance,
                         const std::vector<Point> * const pts = nullptr,
                         const std::vector<Real> * const weights = nullptr);

  /**
   * Reinitialize FE data for the given neighbor_element on the given side with a given set of
   * reference points
   */
  void reinitNeighborFaceRef(const Elem * neighbor_elem,
                             unsigned int neighbor_side,
                             Real tolerance,
                             const std::vector<Point> * const pts,
                             const std::vector<Real> * const weights = nullptr);

  /**
   * Reinitialize FE data for a lower dimenesional element with a given set of reference points
   */
  void reinitLowerDElemRef(const Elem * elem,
                           const std::vector<Point> * const pts,
                           const std::vector<Real> * const weights = nullptr);

  /**
   * reinitialize a mortar segment mesh element in order to get a proper JxW
   */
  void reinitMortarElem(const Elem * elem);

  /**
   * Returns a reference to JxW for mortar segment elements
   */
  const std::vector<Real> & jxWMortar() const { return *_JxW_msm; }

  /**
   * Returns a reference to the quadrature rule for the mortar segments
   */
  const QBase * const & qRuleMortar() const { return _const_qrule_msm; }

private:
  /**
   * compute AD things on an element face
   */
  void computeADFace(const Elem * elem, unsigned int side);

public:
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
   * Reinitialize the assembly data on the side of a element at the custom reference points
   */
  void reinit(const Elem * elem, unsigned int side, const std::vector<Point> & reference_points);

  /**
   * Reinitialize an element and its neighbor along a particular side.
   *
   * @param elem Element being reinitialized
   * @param side Side of the element
   * @param neighbor Neighbor facing the element on the side 'side'
   * @param neighbor_side The side id on the neighboring element.
   */
  void reinitElemAndNeighbor(const Elem * elem,
                             unsigned int side,
                             const Elem * neighbor,
                             unsigned int neighbor_side);

  /**
   * Reinitializes the neighbor at the physical coordinates on neighbor side given.
   */
  void reinitNeighborAtPhysical(const Elem * neighbor,
                                unsigned int neighbor_side,
                                const std::vector<Point> & physical_points);

  /**
   * Reinitializes the neighbor at the physical coordinates within element given.
   */
  void reinitNeighborAtPhysical(const Elem * neighbor, const std::vector<Point> & physical_points);

  void reinitNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  /**
   * Reinitialize assembly data for a node
   */
  void reinit(const Node * node);

  /**
   * Initialize the Assembly object and set the CouplingMatrix for use throughout.
   */
  void init(const CouplingMatrix * cm);

  /// Create pair of variables requiring nonlocal jacobian contributions
  void initNonlocalCoupling();

  /// Sizes and zeroes the Jacobian blocks used for the current element
  void prepareJacobianBlock();

  /// Sizes and zeroes the residual for the current element
  void prepareResidual();

  void prepare();
  void prepareNonlocal();

  /**
   * Used for preparing the dense residual and jacobian blocks for one particular variable.
   *
   * @param var The variable that needs to have its datastructures prepared
   */
  void prepareVariable(MooseVariableFEBase * var);
  void prepareVariableNonlocal(MooseVariableFEBase * var);
  void prepareNeighbor();

  /**
   * Prepare the Jacobians and residuals for a lower dimensional element. This method may be called
   * when performing mortar finite element simulations
   */
  void prepareLowerD();

  void prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<dof_id_type> & dof_indices);
  void prepareBlockNonlocal(unsigned int ivar,
                            unsigned jvar,
                            const std::vector<dof_id_type> & idof_indices,
                            const std::vector<dof_id_type> & jdof_indices);
  void prepareScalar();
  void prepareOffDiagScalar();

  template <typename T>
  void copyShapes(MooseVariableFE<T> & v);
  void copyShapes(unsigned int var);

  template <typename T>
  void copyFaceShapes(MooseVariableFE<T> & v);
  void copyFaceShapes(unsigned int var);

  template <typename T>
  void copyNeighborShapes(MooseVariableFE<T> & v);
  void copyNeighborShapes(unsigned int var);

  /**
   * Add local residuals of all field variables for a tag onto a given global residual vector.
   */
  void addResidual(NumericVector<Number> & residual, TagID tag_id = 0);
  /**
   * Add local residuals of all field variables for a set of tags onto the global residual vectors
   * associated with the tags.
   */
  void addResidual(const std::map<TagName, TagID> & tags);
  /**
   * Add local neighbor residuals of all field variables for a tag onto a given global residual
   * vector.
   */
  void addResidualNeighbor(NumericVector<Number> & residual, TagID tag_id = 0);
  /**
   * Add local neighbor residuals of all field variables for a set of tags onto the global residual
   * vectors associated with the tags.
   */
  void addResidualNeighbor(const std::map<TagName, TagID> & tags);
  /**
   * Add residuals of all scalar variables for a tag onto the global residual vector associated
   * with the tag.
   */
  void addResidualScalar(TagID tag_id);
  /**
   * Add residuals of all scalar variables for a set of tags onto the global residual vectors
   * associated with the tags.
   */
  void addResidualScalar(const std::map<TagName, TagID> & tags);

  /**
   * Takes the values that are currently in _sub_Re of all field variables and appends them to
   * the cached values.
   */
  void cacheResidual();

  /**
   * Cache individual residual contributions.  These will ultimately get added to the residual when
   * addCachedResidual() is called.
   *
   * @param dof The degree of freedom to add the residual contribution to
   * @param value The value of the residual contribution.
   * @param TagID  the contribution should go to the tagged residual
   */
  void cacheResidualContribution(dof_id_type dof, Real value, TagID tag_id);

  /**
   * Cache individual residual contributions.  These will ultimately get added to the residual when
   * addCachedResidual() is called.
   *
   * @param dof The degree of freedom to add the residual contribution to
   * @param value The value of the residual contribution.
   * @param tags the contribution should go to all tags
   */
  void cacheResidualContribution(dof_id_type dof, Real value, const std::set<TagID> & tags);

  /**
   * Lets an external class cache residual at a set of nodes
   */
  void cacheResidualNodes(const DenseVector<Number> & res,
                          const std::vector<dof_id_type> & dof_index,
                          TagID tag = 0);

  /**
   * Takes the values that are currently in _sub_Rn of all field variables and appends them to
   * the cached values.
   */
  void cacheResidualNeighbor();

  /**
   * Takes the values that are currently in _sub_Rl and appends them to the cached values.
   */
  void cacheResidualLower();

  /**
   * Pushes all cached residuals to the global residual vectors.
   */
  void addCachedResiduals();

  /**
   * Adds the values that have been cached by calling cacheResidual(), cacheResidualNeighbor(),
   * and/or cacheResidualLower() to the residual.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidual(NumericVector<Number> & residual, TagID tag_id);

  /**
   * Sets local residuals of all field variables to the global residual vector for a tag.
   */
  void setResidual(NumericVector<Number> & residual, TagID tag_id = 0);

  /**
   * Sets local neighbor residuals of all field variables to the global residual vector for a tag.
   */
  void setResidualNeighbor(NumericVector<Number> & residual, TagID tag_id = 0);

  /**
   * Adds all local Jacobian to the global Jacobian matrices.
   */
  void addJacobian();

  /**
   * Adds non-local Jacobian to the global Jacobian matrices.
   */
  void addJacobianNonlocal();

  /**
   * Add ElementNeighbor, NeighborElement, and NeighborNeighbor portions of the Jacobian for compute
   * objects like DGKernels
   */
  void addJacobianNeighbor();

  /**
   * Add Jacobians for pairs of scalar variables into the global Jacobian matrices.
   */
  void addJacobianScalar();

  /**
   * Add Jacobians for a scalar variables with all other field variables into the global Jacobian
   * matrices.
   */
  void addJacobianOffDiagScalar(unsigned int ivar);

  /**
   * Adds element matrix for ivar rows and jvar columns to the global Jacobian matrix.
   */
  void addJacobianBlock(SparseMatrix<Number> & jacobian,
                        unsigned int ivar,
                        unsigned int jvar,
                        const DofMap & dof_map,
                        std::vector<dof_id_type> & dof_indices);

  /**
   * Adds non-local element matrix for ivar rows and jvar columns to the global Jacobian matrix.
   */
  void addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                const std::vector<dof_id_type> & idof_indices,
                                const std::vector<dof_id_type> & jdof_indices);

  /**
   * Add LowerLower, LowerSlave (LowerElement), LowerMaster (LowerNeighbor), SlaveLower
   * (ElementLower), and MasterLower (NeighborLower) portions of the Jacobian for compute objects
   * like MortarConstraints
   */
  void addJacobianLower();

  /**
   * Adds three neighboring element matrices for ivar rows and jvar columns to the global Jacobian
   * matrix.
   */
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                           unsigned int ivar,
                           unsigned int jvar,
                           const DofMap & dof_map,
                           std::vector<dof_id_type> & dof_indices,
                           std::vector<dof_id_type> & neighbor_dof_indices);

  /**
   * Takes the values that are currently in _sub_Kee and appends them to the cached values.
   */
  void cacheJacobian();

  /**
   * Caches element matrix for ivar rows and jvar columns
   */
  void cacheJacobianCoupledVarPair(const MooseVariableBase & ivar, const MooseVariableBase & jvar);

  /**
   * Takes the values that are currently in _sub_Keg and appends them to the cached values.
   */
  void cacheJacobianNonlocal();

  /**
   * Takes the values that are currently in the neighbor Dense Matrices and appends them to the
   * cached values.
   */
  void cacheJacobianNeighbor();

  /**
   * Adds the values that have been cached by calling cacheJacobian() and or cacheJacobianNeighbor()
   * to the jacobian matrix.
   *
   * Note that this will also clear the cache.
   */
  void addCachedJacobian();

  /**
   * Same as addCachedJacobian but deprecated.
   */
  void addCachedJacobian(SparseMatrix<Number> & jacobian);

  /**
   * Get local residual block for a variable and a tag.
   */
  DenseVector<Number> & residualBlock(unsigned int var_num, TagID tag_id = 0)
  {
    return _sub_Re[tag_id][var_num];
  }

  /**
   * Get local neighbor residual block for a variable and a tag.
   */
  DenseVector<Number> & residualBlockNeighbor(unsigned int var_num, TagID tag_id = 0)
  {
    return _sub_Rn[tag_id][var_num];
  }

  /**
   * Get residual block for lower.
   */
  DenseVector<Number> & residualBlockLower(unsigned int var_num, TagID tag_id = 0)
  {
    return _sub_Rl[tag_id][var_num];
  }

  /**
   * Get local Jacobian block for a pair of variables and a tag.
   */
  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar, TagID tag = 0)
  {
    _jacobian_block_used[tag][ivar][jvar] = 1;
    return _sub_Kee[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Get local Jacobian block from non-local contribution for a pair of variables and a tag.
   */
  DenseMatrix<Number> & jacobianBlockNonlocal(unsigned int ivar, unsigned int jvar, TagID tag = 0)
  {
    _jacobian_block_nonlocal_used[tag][ivar][jvar] = 1;
    return _sub_Keg[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Get local Jacobian block of a DG Jacobian type for a pair of variables and a tag.
   */
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type,
                                              unsigned int ivar,
                                              unsigned int jvar,
                                              TagID tag = 0);

  /**
   * Returns the jacobian block for the given mortar Jacobian type
   */
  DenseMatrix<Number> & jacobianBlockLower(Moose::ConstraintJacobianType type,
                                           unsigned int ivar,
                                           unsigned int jvar,
                                           TagID tag = 0);

  void cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                          const std::vector<dof_id_type> & idof_indices,
                          const std::vector<dof_id_type> & jdof_indices,
                          Real scaling_factor,
                          TagID tag = 0);

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> & couplingEntries()
  {
    return _cm_ff_entry;
  }
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> & nonlocalCouplingEntries()
  {
    return _cm_nonlocal_entry;
  }

  // Read-only references
  const VariablePhiValue & phi() const { return _phi; }
  template <typename T, ComputeStage compute_stage>
  const typename VariableTestGradientType<T, compute_stage>::type &
  adGradPhi(const MooseVariableFE<T> & v) const
  {
    return gradPhi(v);
  }
  const VariablePhiValue & phi(const MooseVariable &) const { return _phi; }
  const VariablePhiGradient & gradPhi() const { return _grad_phi; }
  const VariablePhiGradient & gradPhi(const MooseVariable &) const { return _grad_phi; }
  const VariablePhiSecond & secondPhi() const { return _second_phi; }
  const VariablePhiSecond & secondPhi(const MooseVariable &) const { return _second_phi; }

  const VariablePhiValue & phiFace() const { return _phi_face; }
  const VariablePhiValue & phiFace(const MooseVariable &) const { return _phi_face; }
  const VariablePhiGradient & gradPhiFace() const { return _grad_phi_face; }
  const VariablePhiGradient & gradPhiFace(const MooseVariable &) const { return _grad_phi_face; }
  const VariablePhiSecond & secondPhiFace(const MooseVariable &) const { return _second_phi_face; }

  const VariablePhiValue & phiNeighbor(const MooseVariable &) const { return _phi_neighbor; }
  const VariablePhiGradient & gradPhiNeighbor(const MooseVariable &) const
  {
    return _grad_phi_neighbor;
  }
  const VariablePhiSecond & secondPhiNeighbor(const MooseVariable &) const
  {
    return _second_phi_neighbor;
  }

  const VariablePhiValue & phiFaceNeighbor(const MooseVariable &) const
  {
    return _phi_face_neighbor;
  }
  const VariablePhiGradient & gradPhiFaceNeighbor(const MooseVariable &) const
  {
    return _grad_phi_face_neighbor;
  }
  const VariablePhiSecond & secondPhiFaceNeighbor(const MooseVariable &) const
  {
    return _second_phi_face_neighbor;
  }

  const VectorVariablePhiValue & phi(const VectorMooseVariable &) const { return _vector_phi; }
  const VectorVariablePhiGradient & gradPhi(const VectorMooseVariable &) const
  {
    return _vector_grad_phi;
  }
  const VectorVariablePhiSecond & secondPhi(const VectorMooseVariable &) const
  {
    return _vector_second_phi;
  }
  const VectorVariablePhiCurl & curlPhi(const VectorMooseVariable &) const
  {
    return _vector_curl_phi;
  }

  const VectorVariablePhiValue & phiFace(const VectorMooseVariable &) const
  {
    return _vector_phi_face;
  }
  const VectorVariablePhiGradient & gradPhiFace(const VectorMooseVariable &) const
  {
    return _vector_grad_phi_face;
  }
  const VectorVariablePhiSecond & secondPhiFace(const VectorMooseVariable &) const
  {
    return _vector_second_phi_face;
  }
  const VectorVariablePhiCurl & curlPhiFace(const VectorMooseVariable &) const
  {
    return _vector_curl_phi_face;
  }

  const VectorVariablePhiValue & phiNeighbor(const VectorMooseVariable &) const
  {
    return _vector_phi_neighbor;
  }
  const VectorVariablePhiGradient & gradPhiNeighbor(const VectorMooseVariable &) const
  {
    return _vector_grad_phi_neighbor;
  }
  const VectorVariablePhiSecond & secondPhiNeighbor(const VectorMooseVariable &) const
  {
    return _vector_second_phi_neighbor;
  }
  const VectorVariablePhiCurl & curlPhiNeighbor(const VectorMooseVariable &) const
  {
    return _vector_curl_phi_neighbor;
  }

  const VectorVariablePhiValue & phiFaceNeighbor(const VectorMooseVariable &) const
  {
    return _vector_phi_face_neighbor;
  }
  const VectorVariablePhiGradient & gradPhiFaceNeighbor(const VectorMooseVariable &) const
  {
    return _vector_grad_phi_face_neighbor;
  }
  const VectorVariablePhiSecond & secondPhiFaceNeighbor(const VectorMooseVariable &) const
  {
    return _vector_second_phi_face_neighbor;
  }
  const VectorVariablePhiCurl & curlPhiFaceNeighbor(const VectorMooseVariable &) const
  {
    return _vector_curl_phi_face_neighbor;
  }

  // Writeable references
  VariablePhiValue & phi(const MooseVariable &) { return _phi; }
  VariablePhiGradient & gradPhi(const MooseVariable &) { return _grad_phi; }
  VariablePhiSecond & secondPhi(const MooseVariable &) { return _second_phi; }

  VariablePhiValue & phiFace(const MooseVariable &) { return _phi_face; }
  VariablePhiGradient & gradPhiFace(const MooseVariable &) { return _grad_phi_face; }
  VariablePhiSecond & secondPhiFace(const MooseVariable &) { return _second_phi_face; }

  VariablePhiValue & phiNeighbor(const MooseVariable &) { return _phi_neighbor; }
  VariablePhiGradient & gradPhiNeighbor(const MooseVariable &) { return _grad_phi_neighbor; }
  VariablePhiSecond & secondPhiNeighbor(const MooseVariable &) { return _second_phi_neighbor; }

  VariablePhiValue & phiFaceNeighbor(const MooseVariable &) { return _phi_face_neighbor; }
  VariablePhiGradient & gradPhiFaceNeighbor(const MooseVariable &)
  {
    return _grad_phi_face_neighbor;
  }
  VariablePhiSecond & secondPhiFaceNeighbor(const MooseVariable &)
  {
    return _second_phi_face_neighbor;
  }

  // Writeable references with vector variable
  VectorVariablePhiValue & phi(const VectorMooseVariable &) { return _vector_phi; }
  VectorVariablePhiGradient & gradPhi(const VectorMooseVariable &) { return _vector_grad_phi; }
  VectorVariablePhiSecond & secondPhi(const VectorMooseVariable &) { return _vector_second_phi; }
  VectorVariablePhiCurl & curlPhi(const VectorMooseVariable &) { return _vector_curl_phi; }

  VectorVariablePhiValue & phiFace(const VectorMooseVariable &) { return _vector_phi_face; }
  VectorVariablePhiGradient & gradPhiFace(const VectorMooseVariable &)
  {
    return _vector_grad_phi_face;
  }
  VectorVariablePhiSecond & secondPhiFace(const VectorMooseVariable &)
  {
    return _vector_second_phi_face;
  }
  VectorVariablePhiCurl & curlPhiFace(const VectorMooseVariable &) { return _vector_curl_phi_face; }

  VectorVariablePhiValue & phiNeighbor(const VectorMooseVariable &) { return _vector_phi_neighbor; }
  VectorVariablePhiGradient & gradPhiNeighbor(const VectorMooseVariable &)
  {
    return _vector_grad_phi_neighbor;
  }
  VectorVariablePhiSecond & secondPhiNeighbor(const VectorMooseVariable &)
  {
    return _vector_second_phi_neighbor;
  }
  VectorVariablePhiCurl & curlPhiNeighbor(const VectorMooseVariable &)
  {
    return _vector_curl_phi_neighbor;
  }
  VectorVariablePhiValue & phiFaceNeighbor(const VectorMooseVariable &)
  {
    return _vector_phi_face_neighbor;
  }
  VectorVariablePhiGradient & gradPhiFaceNeighbor(const VectorMooseVariable &)
  {
    return _vector_grad_phi_face_neighbor;
  }
  VectorVariablePhiSecond & secondPhiFaceNeighbor(const VectorMooseVariable &)
  {
    return _vector_second_phi_face_neighbor;
  }
  VectorVariablePhiCurl & curlPhiFaceNeighbor(const VectorMooseVariable &)
  {
    return _vector_curl_phi_face_neighbor;
  }

  // Writeable references with array variable
  VariablePhiValue & phi(const ArrayMooseVariable &) { return _phi; }
  VariablePhiGradient & gradPhi(const ArrayMooseVariable &) { return _grad_phi; }
  VariablePhiSecond & secondPhi(const ArrayMooseVariable &) { return _second_phi; }

  VariablePhiValue & phiFace(const ArrayMooseVariable &) { return _phi_face; }
  VariablePhiGradient & gradPhiFace(const ArrayMooseVariable &) { return _grad_phi_face; }
  VariablePhiSecond & secondPhiFace(const ArrayMooseVariable &) { return _second_phi_face; }

  VariablePhiValue & phiNeighbor(const ArrayMooseVariable &) { return _phi_neighbor; }
  VariablePhiGradient & gradPhiNeighbor(const ArrayMooseVariable &) { return _grad_phi_neighbor; }
  VariablePhiSecond & secondPhiNeighbor(const ArrayMooseVariable &) { return _second_phi_neighbor; }

  VariablePhiValue & phiFaceNeighbor(const ArrayMooseVariable &) { return _phi_face_neighbor; }
  VariablePhiGradient & gradPhiFaceNeighbor(const ArrayMooseVariable &)
  {
    return _grad_phi_face_neighbor;
  }
  VariablePhiSecond & secondPhiFaceNeighbor(const ArrayMooseVariable &)
  {
    return _second_phi_face_neighbor;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhi(FEType type) const
  {
    buildFE(type);
    return _fe_shape_data[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhi(FEType type) const
  {
    buildFE(type);
    return _fe_shape_data[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type &
  feADGradPhi(FEType type) const
  {
    return _ad_grad_phi_data[type];
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhi(FEType type) const
  {
    _need_second_derivative[type] = true;
    buildFE(type);
    return _fe_shape_data[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiLower(FEType type) const;

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiLower(FEType type) const;

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiFace(FEType type) const
  {
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiFace(FEType type) const
  {
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type &
  feADGradPhiFace(FEType type) const
  {
    return _ad_grad_phi_data_face[type];
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhiFace(FEType type) const
  {
    _need_second_derivative[type] = true;
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiNeighbor(FEType type) const
  {
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiNeighbor(FEType type) const
  {
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhiNeighbor(FEType type) const
  {
    _need_second_derivative_neighbor[type] = true;
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiFaceNeighbor(FEType type) const
  {
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient &
  feGradPhiFaceNeighbor(FEType type) const
  {
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond &
  feSecondPhiFaceNeighbor(FEType type) const
  {
    _need_second_derivative_neighbor[type] = true;
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhi(FEType type) const
  {
    _need_curl[type] = true;
    buildFE(type);
    return _fe_shape_data[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiFace(FEType type) const
  {
    _need_curl[type] = true;
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiNeighbor(FEType type) const
  {
    _need_curl[type] = true;
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiFaceNeighbor(FEType type) const
  {
    _need_curl[type] = true;
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_curl_phi;
  }

  /**
   * Caches the Jacobian entry 'value', to eventually be
   * added/set in the (i,j) location of the matrix.
   *
   * We use numeric_index_type for the index arrays (rather than
   * dof_id_type) since that is what the SparseMatrix interface uses,
   * but at the time of this writing, those two types are equivalent.
   */
  void
  cacheJacobianContribution(numeric_index_type i, numeric_index_type j, Real value, TagID tag = 0);

  void cacheJacobianContribution(numeric_index_type i,
                                 numeric_index_type j,
                                 Real value,
                                 const std::set<TagID> & tags);

  /**
   * Sets previously-cached Jacobian values via SparseMatrix::set() calls.
   */
  void setCachedJacobianContributions();

  /**
   * Zero out previously-cached Jacobian rows.
   */
  void zeroCachedJacobianContributions();

  /**
   * Adds previously-cached Jacobian values via SparseMatrix::add() calls.
   */
  void addCachedJacobianContributions();

  /**
   * Set the pointer to the XFEM controller object
   */
  void setXFEM(std::shared_ptr<XFEMInterface> xfem) { _xfem = xfem; }

  void assignDisplacements(std::vector<unsigned> && disp_numbers) { _displacements = disp_numbers; }

  /**
   * Helper function for assembling residual contriubutions on local
   * quadrature points for an array variable
   * @param re The local residual
   * @param i The local test function index
   * @param ntest The number of test functions
   * @param v The residual contribution on the current qp
   */
  void saveLocalArrayResidual(DenseVector<Number> & re,
                              unsigned int i,
                              unsigned int ntest,
                              const RealEigenVector & v)
  {
    for (unsigned int j = 0; j < v.size(); ++j, i += ntest)
      re(i) += v(j);
  }

  /**
   * Helper function for assembling diagonal Jacobian contriubutions on local
   * quadrature points for an array variable
   * @param ke The local Jacobian
   * @param i The local test function index
   * @param ntest The number of test functions
   * @param j The local shape function index
   * @param nphi The number of shape functions
   * @param v The diagonal Jacobian contribution on the current qp
   */
  void saveDiagLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  unsigned int ivar,
                                  const RealEigenVector & v)
  {
    unsigned int pace = (_component_block_diagonal[ivar] ? 0 : nphi);
    for (unsigned int k = 0; k < v.size(); ++k, i += ntest, j += pace)
      ke(i, j) += v(k);
  }

  /**
   * Helper function for assembling full Jacobian contriubutions on local
   * quadrature points for an array variable
   * @param ke The local Jacobian
   * @param i The local test function index
   * @param ntest The number of test functions
   * @param j The local shape function index
   * @param nphi The number of shape functions
   * @param ivar The array variable index
   * @param jvar The contributing variable index
   * @param v The full Jacobian contribution from a variable on the current qp
   */
  void saveFullLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  unsigned int ivar,
                                  unsigned int jvar,
                                  const RealEigenMatrix & v)
  {
    unsigned int pace = ((ivar == jvar && _component_block_diagonal[ivar]) ? 0 : nphi);
    unsigned int saved_j = j;
    for (unsigned int k = 0; k < v.rows(); ++k, i += ntest)
    {
      j = saved_j;
      for (unsigned int l = 0; l < v.cols(); ++l, j += pace)
        ke(i, j) += v(k, l);
    }
  }

  DenseVector<Real> getJacobianDiagonal(DenseMatrix<Number> & ke)
  {
    unsigned int rows = ke.m();
    unsigned int cols = ke.n();
    DenseVector<Real> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      // % operation is needed to account for cases of no component coupling of array variables
      diag(i) = ke(i, i % cols);
    return diag;
  }

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

  void computeFaceMap(unsigned dim, const std::vector<Real> & qw, const Elem * side);

  void reinitFEFaceNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  void reinitFENeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  template <ComputeStage compute_stage>
  void setCoordinateTransformation(const QBase * qrule,
                                   const ADPoint & q_points,
                                   MooseArray<ADReal> & coord,
                                   SubdomainID sub_id);

  void computeCurrentElemVolume();

  void computeCurrentFaceVolume();

  void computeCurrentNeighborVolume();

  /**
   * Appling scaling, constraints to the local residual block and populate the full DoF indices
   * for array variable.
   */
  void processLocalResidual(DenseVector<Number> & res_block,
                            std::vector<dof_id_type> & dof_indices,
                            const std::vector<Real> & scaling_factor,
                            bool is_nodal);
  /**
   * Add a local residual block to a global residual vector with proper scaling.
   */
  void addResidualBlock(NumericVector<Number> & residual,
                        DenseVector<Number> & res_block,
                        const std::vector<dof_id_type> & dof_indices,
                        const std::vector<Real> & scaling_factor,
                        bool is_nodal);
  /**
   * Push a local residual block with proper scaling into cache.
   */
  void cacheResidualBlock(std::vector<Real> & cached_residual_values,
                          std::vector<dof_id_type> & cached_residual_rows,
                          DenseVector<Number> & res_block,
                          const std::vector<dof_id_type> & dof_indices,
                          const std::vector<Real> & scaling_factor,
                          bool is_nodal);

  /**
   * Set a local residual block to a global residual vector with proper scaling.
   */
  void setResidualBlock(NumericVector<Number> & residual,
                        DenseVector<Number> & res_block,
                        const std::vector<dof_id_type> & dof_indices,
                        const std::vector<Real> & scaling_factor,
                        bool is_nodal);

  /**
   * Add a local Jacobian block to a global Jacobian with proper scaling.
   */
  void addJacobianBlock(SparseMatrix<Number> & jacobian,
                        DenseMatrix<Number> & jac_block,
                        const MooseVariableBase & ivar,
                        const MooseVariableBase & jvar,
                        const std::vector<dof_id_type> & idof_indices,
                        const std::vector<dof_id_type> & jdof_indices);

  /**
   * Push a local Jacobian block with proper scaling into cache for a certain tag.
   */
  void cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                          const MooseVariableBase & ivar,
                          const MooseVariableBase & jvar,
                          const std::vector<dof_id_type> & idof_indices,
                          const std::vector<dof_id_type> & jdof_indices,
                          TagID tag = 0);

  /**
   * Push non-zeros of a local Jacobian block with proper scaling into cache for a certain tag.
   */
  void cacheJacobianBlockNonzero(DenseMatrix<Number> & jac_block,
                                 const MooseVariableBase & ivar,
                                 const MooseVariableBase & jvar,
                                 const std::vector<dof_id_type> & idof_indices,
                                 const std::vector<dof_id_type> & jdof_indices,
                                 TagID tag = 0);

  /**
   * Adds element matrices for ivar rows and jvar columns to the global Jacobian matrices.
   */
  void addJacobianCoupledVarPair(const MooseVariableBase & ivar, const MooseVariableBase & jvar);

  /**
   * Clear any currently cached jacobian contributions
   *
   * This is automatically called by setCachedJacobianContributions and
   * addCachedJacobianContributions
   */
  void clearCachedJacobianContributions();

  /**
   * Update the integration weights for XFEM partial elements.
   * This only affects the weights if XFEM is used and if the element is cut.
   * @param elem The element for which the weights are adjusted
   */
  void modifyWeightsDueToXFEM(const Elem * elem);

  /**
   * Update the face integration weights for XFEM partial elements.
   * This only affects the weights if XFEM is used and if the element is cut.
   * @param elem The element for which the weights are adjusted
   * @param side The side of element for which the weights are adjusted
   */
  void modifyFaceWeightsDueToXFEM(const Elem * elem, unsigned int side = 0);

  template <typename OutputType>
  void computeGradPhiAD(
      const Elem * elem,
      unsigned int n_qp,
      typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type & grad_phi,
      FEGenericBase<OutputType> * fe);
  void resizeADMappingObjects(unsigned int n_qp, unsigned int dim);
  void computeAffineMapAD(const Elem * elem,
                          const std::vector<Real> & qw,
                          unsigned int n_qp,
                          FEBase * fe);
  void
  computeSinglePointMapAD(const Elem * elem, const std::vector<Real> & qw, unsigned p, FEBase * fe);

private:
  /**
   * Build FEs with a type
   * @param type The type of FE
   */
  void buildFE(FEType type) const;

  /**
   * Build FEs for a face with a type
   * @param type The type of FE
   */
  void buildFaceFE(FEType type) const;

  /**
   * Build FEs for a neighbor with a type
   * @param type The type of FE
   */
  void buildNeighborFE(FEType type) const;

  /**
   * Build FEs for a neighbor face with a type
   * @param type The type of FE
   */
  void buildFaceNeighborFE(FEType type) const;

  /**
   * Build FEs for a lower dimensional element with a type
   * @param type The type of FE
   */
  void buildLowerDFE(FEType type) const;

  /**
   * Build Vector FEs with a type
   * @param type The type of FE
   */
  void buildVectorFE(FEType type) const;

  /**
   * Build Vector FEs for a face with a type
   * @param type The type of FE
   */
  void buildVectorFaceFE(FEType type) const;

  /**
   * Build Vector FEs for a neighbor with a type
   * @param type The type of FE
   */
  void buildVectorNeighborFE(FEType type) const;

  /**
   * Build Vector FEs for a neighbor face with a type
   * @param type The type of FE
   */
  void buildVectorFaceNeighborFE(FEType type) const;

  /**
   * Build Vector FEs for a lower dimensional element with a type
   * @param type The type of FE
   */
  void buildVectorLowerDFE(FEType type) const;

private:
  SystemBase & _sys;
  SubProblem & _subproblem;

  const bool _displaced;

  /// Coupling matrices
  const CouplingMatrix * _cm;
  const CouplingMatrix & _nonlocal_cm;

  const bool & _computing_jacobian;

  /// Entries in the coupling matrix for field variables
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> _cm_ff_entry;
  /// Entries in the coupling matrix for field variables vs scalar variables
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableScalar *>> _cm_fs_entry;
  /// Entries in the coupling matrix for scalar variables vs field variables
  std::vector<std::pair<MooseVariableScalar *, MooseVariableFEBase *>> _cm_sf_entry;
  /// Entries in the coupling matrix for scalar variables
  std::vector<std::pair<MooseVariableScalar *, MooseVariableScalar *>> _cm_ss_entry;
  /// Entries in the coupling matrix for field variables for nonlocal calculations
  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> _cm_nonlocal_entry;
  /// Flag that indicates if the jacobian block was used
  std::vector<std::vector<std::vector<unsigned char>>> _jacobian_block_used;
  std::vector<std::vector<std::vector<unsigned char>>> _jacobian_block_nonlocal_used;
  /// Flag that indicates if the jacobian block for neighbor was used
  std::vector<std::vector<std::vector<unsigned char>>> _jacobian_block_neighbor_used;
  /// Flag that indicates if the jacobian block for the lower dimensional element was used
  std::vector<std::vector<std::vector<unsigned char>>> _jacobian_block_lower_used;
  /// DOF map
  const DofMap & _dof_map;
  /// Thread number (id)
  THREAD_ID _tid;

  MooseMesh & _mesh;

  unsigned int _mesh_dimension;

  /// The XFEM controller
  std::shared_ptr<XFEMInterface> _xfem;

  /// The "volume" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe;
  /// The "face" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe_face;
  /// The "neighbor" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe_neighbor;
  /// The "neighbor face" fe object that matches the current elem
  std::map<FEType, FEBase *> _current_fe_face_neighbor;

  /// The "volume" vector fe object that matches the current elem
  std::map<FEType, FEVectorBase *> _current_vector_fe;
  /// The "face" vector fe object that matches the current elem
  std::map<FEType, FEVectorBase *> _current_vector_fe_face;
  /// The "neighbor" vector fe object that matches the current elem
  std::map<FEType, FEVectorBase *> _current_vector_fe_neighbor;
  /// The "neighbor face" vector fe object that matches the current elem
  std::map<FEType, FEVectorBase *> _current_vector_fe_face_neighbor;

  /**** Volume Stuff ****/

  /// Each dimension's actual fe objects indexed on type
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe;
  /// Each dimension's actual fe objects indexed on type
  mutable std::map<unsigned int, std::map<FEType, const FEBase *>> _const_fe;
  /// Each dimension's actual vector fe objects indexed on type
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe;
  /// Each dimension's actual vector fe objects indexed on type
  mutable std::map<unsigned int, std::map<FEType, const FEVectorBase *>> _const_vector_fe;
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_helper;
  /// The current helper object for transforming coordinates
  FEBase * _current_fe_helper;
  /// The current current quadrature rule being used (could be either volumetric or arbitrary - for dirac kernels)
  const QBase * _const_current_qrule;
  /// The current current quadrature rule being used (could be either volumetric or arbitrary - for dirac kernels)
  QBase * _current_qrule;
  /// The current volumetric quadrature for the element
  QBase * _current_qrule_volume;
  /// The current arbitrary quadrature rule used within the element interior
  ArbitraryQuadrature * _current_qrule_arbitrary;
  /// The current arbitrary quadrature rule used on the element face
  ArbitraryQuadrature * _current_qrule_arbitrary_face;
  /// The current list of quadrature points
  MooseArray<Point> _current_q_points;
  /// The current list of transformed jacobian weights
  MooseArray<Real> _current_JxW;
  /// The coordinate system
  Moose::CoordinateSystemType _coord_type;
  /// The current coordinate transformation coefficients
  MooseArray<Real> _coord;
  /// The AD version of the current coordinate transformation coefficients
  MooseArray<DualReal> _ad_coord;

  /// Holds volume qrules for each dimension
  std::map<unsigned int, QBase *> _holder_qrule_volume;
  /// Holds arbitrary qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_arbitrary;
  /// Holds arbitrary qrules for each dimension for faces
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_arbitrary_face;
  /// Holds pointers to the dimension's q_points
  std::map<unsigned int, const std::vector<Point> *> _holder_q_points;
  /// Holds pointers to the dimension's transformed jacobian weights
  std::map<unsigned int, const std::vector<Real> *> _holder_JxW;

  /**** Face Stuff ****/

  /// types of finite elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face;
  /// types of finite elements
  mutable std::map<unsigned int, std::map<FEType, const FEBase *>> _const_fe_face;
  /// types of vector finite elements
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face;
  /// types of vector finite elements
  mutable std::map<unsigned int, std::map<FEType, const FEVectorBase *>> _const_vector_fe_face;
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_face_helper;
  /// helper object for transforming coordinates
  FEBase * _current_fe_face_helper;
  /// quadrature rule used on faces
  const QBase * _const_current_qrule_face;
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
  /// Mapped normals
  std::vector<Eigen::Map<RealDIMValue>> _mapped_normals;
  /// The current tangent vectors at the quadrature points
  MooseArray<std::vector<Point>> _current_tangents;
  /// Holds face qrules for each dimension
  std::map<unsigned int, QBase *> _holder_qrule_face;
  /// Holds pointers to the dimension's q_points on a face
  std::map<unsigned int, const std::vector<Point> *> _holder_q_points_face;
  /// Holds pointers to the dimension's transformed jacobian weights on a face
  std::map<unsigned int, const std::vector<Real> *> _holder_JxW_face;
  /// Holds pointers to the dimension's normal vectors
  std::map<unsigned int, const std::vector<Point> *> _holder_normals;

  /**** Neighbor Stuff ****/

  /// types of finite elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face_neighbor;
  mutable std::map<unsigned int, std::map<FEType, const FEBase *>> _const_fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, const FEBase *>> _const_fe_face_neighbor;
  mutable std::map<unsigned int, std::map<FEType, const FEVectorBase *>> _const_vector_fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, const FEVectorBase *>>
      _const_vector_fe_face_neighbor;

  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_neighbor_helper;
  std::map<unsigned int, FEBase **> _holder_fe_face_neighbor_helper;

  /// FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_lower;
  /// FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, const FEBase *>> _const_fe_lower;
  /// Vector FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_lower;
  /// Vector FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, const FEVectorBase *>> _const_vector_fe_lower;

  /// quadrature rule used on neighbors
  const QBase * _const_current_qrule_neighbor;
  /// quadrature rule used on neighbors
  QBase * _current_qrule_neighbor;
  /// The current quadrature points on the neighbor face
  MooseArray<Point> _current_q_points_face_neighbor;
  /// Holds arbitrary qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_neighbor;
  /// The current transformed jacobian weights on a neighbor's face
  MooseArray<Real> _current_JxW_neighbor;
  /// The current coordinate transformation coefficients
  MooseArray<Real> _coord_neighbor;
  /// The coordinate transformation coefficients evaluated on the quadrature points of the mortar
  /// segment mesh
  MooseArray<Real> _coord_msm;

  /********** mortar stuff *************/

  /// A JxW for working on mortar segement elements
  const std::vector<Real> * _JxW_msm;
  /// A FE object for working on mortar segement elements
  std::unique_ptr<FEBase> _fe_msm;
  /// A qrule object for working on mortar segement elements. This needs to be a
  /// raw pointer because we need to be able to return a reference to it because
  /// we will be constructing other objects that need the qrule before the qrule
  /// is actually created
  QBase * _qrule_msm;
  /// A pointer to const qrule_msm
  const QBase * _const_qrule_msm;

  /// The current "element" we are currently on.
  const Elem * _current_elem;
  /// The current subdomain ID
  SubdomainID _current_subdomain_id;
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
  /// The current neighbor subdomain ID
  SubdomainID _current_neighbor_subdomain_id;
  /// The current side of the selected neighboring element (valid only when working with sides)
  unsigned int _current_neighbor_side;
  /// The current side element of the ncurrent neighbor element
  const Elem * _current_neighbor_side_elem;
  /// true is apps need to compute neighbor element volume
  mutable bool _need_neighbor_elem_volume;
  /// Volume of the current neighbor
  Real _current_neighbor_volume;
  /// The current node we are working with
  const Node * _current_node;
  /// The current neighboring node we are working with
  const Node * _current_neighbor_node;
  /// Boolean to indicate whether current element volumes has been computed
  bool _current_elem_volume_computed;
  /// Boolean to indicate whether current element side volumes has been computed
  bool _current_side_volume_computed;

  /// The current lower dimensional element
  const Elem * _current_lower_d_elem;

  /// This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
  MooseArray<Point> _current_physical_points;

  /*
   * Residual contributions <Tag, ivar>
   * When ivar corresponds to an array variable, the dense vector is in size of ndof * count,
   * where count is the number of components of the array variable. The local residual is ordered
   * as (r_i,j, i = 1,...,ndof; j = 1,...,count).
   *
   * Dense vectors for variables (ivar+i, i = 1,...,count) are empty.
   */
  std::vector<std::vector<DenseVector<Number>>> _sub_Re;
  std::vector<std::vector<DenseVector<Number>>> _sub_Rn;
  /// residual contributions for each variable from the lower dimensional element
  std::vector<std::vector<DenseVector<Number>>> _sub_Rl;

  /// auxiliary vector for scaling residuals (optimization to avoid expensive construction/destruction)
  DenseVector<Number> _tmp_Re;

  /*
   * Jacobian contributions <Tag, ivar, jvar>
   * When ivar corresponds to an array variable, the number of rows of the dense matrix is in size
   * of indof * icount, where icount is the number of components of ivar. When jvar corresponds to
   * an array variable, the number of columns of the dense matrix is in size of jndof * jcount,
   * where jcount is the number of components of jvar. The local residual is ordered as
   * (K_(i,j,k,l), k=1,...,jndof; l = 1,...,jcout; i = 1,...,indof; j = 1,...,icount).
   *
   * Dense matrices for variables (ivar+i, i = 1,...,icount) or (jvar+j, j = 1,...,jcount) are
   * empty.
   */
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kee;
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Keg;

  /// jacobian contributions from the element and neighbor <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Ken;
  /// jacobian contributions from the neighbor and element <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kne;
  /// jacobian contributions from the neighbor <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Knn;
  /// dlower/dlower
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kll;
  /// dlower/dslave (or dlower/delement)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kle;
  /// dlower/dmaster (or dlower/dneighbor)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kln;
  /// dslave/dlower (or delement/dlower)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kel;
  /// dmaster/dlower (or dneighbor/dlower)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Knl;

  /// auxiliary matrix for scaling jacobians (optimization to avoid expensive construction/destruction)
  DenseMatrix<Number> _tmp_Ke;

  // Shape function values, gradients. second derivatives
  VariablePhiValue _phi;
  VariablePhiGradient _grad_phi;
  VariablePhiSecond _second_phi;

  VariablePhiValue _phi_face;
  VariablePhiGradient _grad_phi_face;
  VariablePhiSecond _second_phi_face;

  VariablePhiValue _phi_neighbor;
  VariablePhiGradient _grad_phi_neighbor;
  VariablePhiSecond _second_phi_neighbor;

  VariablePhiValue _phi_face_neighbor;
  VariablePhiGradient _grad_phi_face_neighbor;
  VariablePhiSecond _second_phi_face_neighbor;

  // Shape function values, gradients, second derivatives
  VectorVariablePhiValue _vector_phi;
  VectorVariablePhiGradient _vector_grad_phi;
  VectorVariablePhiSecond _vector_second_phi;
  VectorVariablePhiCurl _vector_curl_phi;

  VectorVariablePhiValue _vector_phi_face;
  VectorVariablePhiGradient _vector_grad_phi_face;
  VectorVariablePhiSecond _vector_second_phi_face;
  VectorVariablePhiCurl _vector_curl_phi_face;

  VectorVariablePhiValue _vector_phi_neighbor;
  VectorVariablePhiGradient _vector_grad_phi_neighbor;
  VectorVariablePhiSecond _vector_second_phi_neighbor;
  VectorVariablePhiCurl _vector_curl_phi_neighbor;

  VectorVariablePhiValue _vector_phi_face_neighbor;
  VectorVariablePhiGradient _vector_grad_phi_face_neighbor;
  VectorVariablePhiSecond _vector_second_phi_face_neighbor;
  VectorVariablePhiCurl _vector_curl_phi_face_neighbor;

  class FEShapeData
  {
  public:
    VariablePhiValue _phi;
    VariablePhiGradient _grad_phi;
    VariablePhiSecond _second_phi;
    VariablePhiCurl _curl_phi;
  };

  class VectorFEShapeData
  {
  public:
    VectorVariablePhiValue _phi;
    VectorVariablePhiGradient _grad_phi;
    VectorVariablePhiSecond _second_phi;
    VectorVariablePhiCurl _curl_phi;
  };

  /// Shape function values, gradients, second derivatives for each FE type
  mutable std::map<FEType, FEShapeData *> _fe_shape_data;
  mutable std::map<FEType, FEShapeData *> _fe_shape_data_face;
  mutable std::map<FEType, FEShapeData *> _fe_shape_data_neighbor;
  mutable std::map<FEType, FEShapeData *> _fe_shape_data_face_neighbor;
  mutable std::map<FEType, FEShapeData *> _fe_shape_data_lower;

  /// Shape function values, gradients, second derivatives for each vector FE type
  mutable std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data;
  mutable std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_face;
  mutable std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_neighbor;
  mutable std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_face_neighbor;
  mutable std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_lower;

  mutable std::map<FEType, typename VariableTestGradientType<Real, ComputeStage::JACOBIAN>::type>
      _ad_grad_phi_data;
  mutable std::map<FEType,
                   typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type>
      _ad_vector_grad_phi_data;
  mutable std::map<FEType, typename VariableTestGradientType<Real, ComputeStage::JACOBIAN>::type>
      _ad_grad_phi_data_face;
  mutable std::map<FEType,
                   typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type>
      _ad_vector_grad_phi_data_face;

  /// Values cached by calling cacheResidual() (the first vector is for TIME vs NONTIME)
  std::vector<std::vector<Real>> _cached_residual_values;

  /// Where the cached values should go (the first vector is for TIME vs NONTIME)
  std::vector<std::vector<dof_id_type>> _cached_residual_rows;

  unsigned int _max_cached_residuals;

  /// Values cached by calling cacheJacobian()
  std::vector<std::vector<Real>> _cached_jacobian_values;
  /// Row where the corresponding cached value should go
  std::vector<std::vector<dof_id_type>> _cached_jacobian_rows;
  /// Column where the corresponding cached value should go
  std::vector<std::vector<dof_id_type>> _cached_jacobian_cols;

  unsigned int _max_cached_jacobians;

  /// Will be true if our preconditioning matrix is a block-diagonal matrix.  Which means that we can take some shortcuts.
  bool _block_diagonal_matrix;
  /// An flag array Indiced by variable index to show if there is no component-wise
  /// coupling for the variable.
  std::vector<bool> _component_block_diagonal;

  /// Temporary work vector to keep from reallocating it
  std::vector<dof_id_type> _temp_dof_indices;

  /// Temporary work data for reinitAtPhysical()
  std::vector<Point> _temp_reference_points;

  /**
   * Storage for cached Jacobian entries
   */
  std::vector<std::vector<Real>> _cached_jacobian_contribution_vals;
  std::vector<std::vector<numeric_index_type>> _cached_jacobian_contribution_rows;
  std::vector<std::vector<numeric_index_type>> _cached_jacobian_contribution_cols;

  /// AD quantities
  std::vector<VectorValue<DualReal>> _ad_dxyzdxi_map;
  std::vector<VectorValue<DualReal>> _ad_dxyzdeta_map;
  std::vector<VectorValue<DualReal>> _ad_dxyzdzeta_map;
  std::vector<VectorValue<DualReal>> _ad_d2xyzdxi2_map;
  std::vector<VectorValue<DualReal>> _ad_d2xyzdxideta_map;
  std::vector<VectorValue<DualReal>> _ad_d2xyzdeta2_map;
  std::vector<DualReal> _ad_jac;
  MooseArray<DualReal> _ad_JxW;
  MooseArray<VectorValue<DualReal>> _ad_q_points;
  std::vector<DualReal> _ad_dxidx_map;
  std::vector<DualReal> _ad_dxidy_map;
  std::vector<DualReal> _ad_dxidz_map;
  std::vector<DualReal> _ad_detadx_map;
  std::vector<DualReal> _ad_detady_map;
  std::vector<DualReal> _ad_detadz_map;
  std::vector<DualReal> _ad_dzetadx_map;
  std::vector<DualReal> _ad_dzetady_map;
  std::vector<DualReal> _ad_dzetadz_map;

  MooseArray<DualReal> _ad_JxW_face;
  MooseArray<VectorValue<DualReal>> _ad_normals;
  MooseArray<VectorValue<DualReal>> _ad_q_points_face;
  MooseArray<Real> _curvatures;
  MooseArray<DualReal> _ad_curvatures;

  std::vector<unsigned> _displacements;

  mutable bool _calculate_xyz;
  mutable bool _calculate_face_xyz;
  mutable bool _calculate_curvatures;

  mutable std::map<FEType, bool> _need_second_derivative;
  mutable std::map<FEType, bool> _need_second_derivative_neighbor;
  mutable std::map<FEType, bool> _need_curl;
};

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiValue &
Assembly::fePhiLower(FEType type) const
{
  buildLowerDFE(type);
  return _fe_shape_data_lower[type]->_phi;
}

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiGradient &
Assembly::feGradPhiLower(FEType type) const
{
  buildLowerDFE(type);
  return _fe_shape_data_lower[type]->_grad_phi;
}

template <>
inline const typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type &
Assembly::feADGradPhi<RealVectorValue>(FEType type) const
{
  return _ad_vector_grad_phi_data[type];
}

template <>
inline const typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type &
Assembly::feADGradPhiFace<RealVectorValue>(FEType type) const
{
  return _ad_vector_grad_phi_data_face[type];
}

template <>
inline void
Assembly::computeGradPhiAD<RealVectorValue>(
    const Elem *,
    unsigned int,
    typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type &,
    FEGenericBase<RealVectorValue> *)
{
  mooseError("Not implemented");
}

template <>
inline const MooseArray<typename Moose::RealType<ComputeStage::JACOBIAN>::type> &
Assembly::adCurvatures<ComputeStage::JACOBIAN>() const
{
  _calculate_curvatures = true;
  return _ad_curvatures;
}

template <>
inline const MooseArray<VectorValue<DualReal>> &
Assembly::adNormals<ComputeStage::JACOBIAN>() const
{
  return _ad_normals;
}

template <>
inline const typename PointType<ComputeStage::JACOBIAN>::type &
Assembly::adQPoints<ComputeStage::JACOBIAN>() const
{
  _calculate_xyz = true;
  return _ad_q_points;
}

template <>
inline const typename PointType<ComputeStage::JACOBIAN>::type &
Assembly::adQPointsFace<ComputeStage::JACOBIAN>() const
{
  _calculate_face_xyz = true;
  return _ad_q_points_face;
}

template <>
inline const MooseArray<DualReal> &
Assembly::adJxWFace<ComputeStage::JACOBIAN>() const
{
  return _ad_JxW_face;
}

template <>
inline const MooseArray<DualReal> &
Assembly::adCoordTransformation<ComputeStage::JACOBIAN>() const
{
  _calculate_xyz = _calculate_face_xyz = true;
  return _ad_coord;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhi<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhi<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhi<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiLower<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiLower<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFace<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFace<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFace<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFaceNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFaceNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFaceNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhi<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFace<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFaceNeighbor<VectorValue<Real>>(FEType type) const;

template <>
const typename VariableTestGradientType<Real, ComputeStage::JACOBIAN>::type &
Assembly::adGradPhi<Real, ComputeStage::JACOBIAN>(const MooseVariableFE<Real> & v) const;

template <>
const typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type &
Assembly::adGradPhi<RealVectorValue, ComputeStage::JACOBIAN>(
    const MooseVariableFE<RealVectorValue> & v) const;

template <>
const MooseArray<typename Moose::RealType<RESIDUAL>::type> & Assembly::adJxW<RESIDUAL>() const;
