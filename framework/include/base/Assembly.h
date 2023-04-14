//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "MooseArray.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "ArbitraryQuadrature.h"

#include "libmesh/dense_vector.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/point.h"
#include "libmesh/fe_base.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/elem_side_builder.h"

#include "DualRealOps.h"

#include <unordered_map>

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
class FaceInfo;
class MooseMesh;
class ArbitraryQuadrature;
class SystemBase;
class MooseVariableFieldBase;
class MooseVariableBase;
template <typename>
class MooseVariableFE;
class MooseVariableScalar;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;
class XFEMInterface;
class SubProblem;
class NodeFaceConstraint;

/// Computes a conversion multiplier for use when computing integraals for the
/// current coordinate system type.  This allows us to handle cases where we use RZ,
/// spherical, or other non-cartesian coordinate systems. The factor returned
/// by this function should generally be multiplied against all integration
/// terms.  Note that the computed factor is particular to a specific point on
/// the mesh.  The result is stored in the factor argument.  point is the point
/// at which to compute the factor.  point and factor can be either Point and
/// Real or ADPoint and ADReal.
template <typename P, typename C>
void coordTransformFactor(const SubProblem & s,
                          SubdomainID sub_id,
                          const P & point,
                          C & factor,
                          SubdomainID neighbor_sub_id = libMesh::Elem::invalid_subdomain_id);

template <typename P, typename C>
void coordTransformFactor(const MooseMesh & mesh,
                          SubdomainID sub_id,
                          const P & point,
                          C & factor,
                          SubdomainID neighbor_sub_id = libMesh::Elem::invalid_subdomain_id);

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
   * Workaround for C++ compilers thinking they can't just cast a
   * const-reference-to-pointer to const-reference-to-const-pointer
   */
  template <typename T>
  static const T * const & constify_ref(T * const & inref)
  {
    const T * const * ptr = &inref;
    return *ptr;
  }

  /**
   * Get a reference to a pointer that will contain the current volume FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const FEBase * const & getFE(FEType type, unsigned int dim) const
  {
    buildFE(type);
    return constify_ref(_fe[dim][type]);
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
    return constify_ref(_fe_neighbor[dim][type]);
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
    return constify_ref(_fe_face[dim][type]);
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
    return constify_ref(_fe_face_neighbor[dim][type]);
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
    return constify_ref(_vector_fe[dim][type]);
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
    return constify_ref(_vector_fe_neighbor[dim][type]);
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
    return constify_ref(_vector_fe_face[dim][type]);
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
    return constify_ref(_vector_fe_face_neighbor[dim][type]);
  }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  const QBase * const & qRule() const { return constify_ref(_current_qrule); }

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
   * Returns the reference to the mortar segment element quadrature points
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const std::vector<Point> & qPointsMortar() const { return _fe_msm->get_xyz(); }

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

  const MooseArray<ADReal> & adJxW() const { return _ad_JxW; }

  const MooseArray<ADReal> & adJxWFace() const { return _ad_JxW_face; }

  const MooseArray<ADReal> & adCurvatures() const
  {
    _calculate_curvatures = true;
    return _ad_curvatures;
  }

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
  const MooseArray<ADReal> & adCoordTransformation() const
  {
    // Coord values for non-cartesian coordinate systems are functions of the locations of the
    // quadrature points in physical space. We also have no way of knowing whether this was called
    // from a volumetric or face object so we should set both volumetric and face xyz to true
    _calculate_xyz = true;
    _calculate_face_xyz = true;

    _calculate_ad_coord = true;
    return _ad_coord;
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
  const QBase * const & qRuleFace() const { return constify_ref(_current_qrule_face); }

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

  /**
   * Returns the array of neighbor normals for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Point> & neighborNormals() const { return _current_neighbor_normals; }

  /***
   * Returns the array of normals for quadrature points on a current side
   */
  const std::vector<Eigen::Map<RealDIMValue>> & mappedNormals() const { return _mapped_normals; }

  /**
   * Returns the array of tangents for quadrature points on a current side
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<std::vector<Point>> & tangents() const { return _current_tangents; }

  /**
   * Returns an integer ID of the current element given the index associated with the integer
   */
  const dof_id_type & extraElemID(unsigned int id) const { return _extra_elem_ids[id]; }

  /**
   * Returns an integer ID of the current element given the index associated with the integer
   */
  const dof_id_type & extraElemIDNeighbor(unsigned int id) const
  {
    return _neighbor_extra_elem_ids[id];
  }

  const MooseArray<ADPoint> & adNormals() const { return _ad_normals; }

  const MooseArray<ADPoint> & adQPoints() const
  {
    _calculate_xyz = true;
    return _ad_q_points;
  }

  const MooseArray<ADPoint> & adQPointsFace() const
  {
    _calculate_face_xyz = true;
    return _ad_q_points_face;
  }

  template <bool is_ad>
  const MooseArray<MooseADWrapper<Point, is_ad>> & genericQPoints() const;

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
   * Return the current boundary ID
   */
  const BoundaryID & currentBoundaryID() const { return _current_boundary_id; }

  /**
   * set the current boundary ID
   */
  void setCurrentBoundaryID(BoundaryID i) { _current_boundary_id = i; }

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
   * Return the neighboring lower dimensional element
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Elem * const & neighborLowerDElem() const { return _current_neighbor_lower_d_elem; }

  /*
   * @return The current lower-dimensional element volume
   */
  const Real & lowerDElemVolume() const;

  /*
   * @return The current neighbor lower-dimensional element volume
   */
  const Real & neighborLowerDElemVolume() const;

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
  const QBase * const & qRuleNeighbor() const { return constify_ref(_current_qrule_neighbor); }

  /**
   * Returns the reference to the current quadrature being used on a current neighbor
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase * const & writeableQRuleNeighbor() { return _current_qrule_neighbor; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWNeighbor() const;

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
   * Creates block-specific volume, face and arbitrary qrules based on the
   * orders and the flag of whether or not to allow negative qweights passed in.
   * Any quadrature rules specified using this function override those created
   * via in the non-block-specific/global createQRules function. order is used
   * for arbitrary volume quadrature rules, while volume_order and face_order
   * are for elem and face quadrature respectively.
   */
  void createQRules(QuadratureType type,
                    Order order,
                    Order volume_order,
                    Order face_order,
                    SubdomainID block,
                    bool allow_negative_qweights = true);

  /**
   * Increases the element/volume quadrature order for the specified mesh
   * block if and only if the current volume quadrature order is lower.  This
   * works exactly like the bumpAllQRuleOrder function, except it only
   * affects the volume quadrature rule (not face quadrature).
   */
  void bumpVolumeQRuleOrder(Order volume_order, SubdomainID block);

  /**
   * Increases the element/volume and face/area quadrature orders for the specified mesh
   * block if and only if the current volume or face quadrature order is lower.  This
   * can only cause the quadrature level to increase.  If order is
   * lower than or equal to the current volume+face quadrature rule order,
   * then nothing is done (i.e. this function is idempotent).
   */
  void bumpAllQRuleOrder(Order order, SubdomainID block);

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
   * Specifies a custom qrule for integration on mortar segment mesh
   *
   * Used to properly integrate QUAD face elements using quadrature on TRI mortar segment elements.
   * For example, to exactly integrate a FIRST order QUAD element, SECOND order quadrature on TRI
   * mortar segments is needed.
   */
  void setMortarQRule(Order order);

  /**
   * Indicates that dual shape functions are used for mortar constraint
   */
  void activateDual() { _need_dual = true; }

  /**
   * Indicates whether dual shape functions are used (computation is now repeated on each element
   * so expense of computing dual shape functions is no longer trivial)
   */
  bool needDual() const { return _need_dual; }

private:
  /**
   * Set the qrule to be used for lower dimensional integration.
   *
   * @param qrule The qrule you want to set
   * @param dim The spatial dimension of the qrule
   */
  void setLowerQRule(QBase * qrule, unsigned int dim);

public:
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
   * Reintialize dual basis coefficients based on a customized quadrature rule
   */
  void reinitDual(const Elem * elem, const std::vector<Point> & pts, const std::vector<Real> & JxW);

  /**
   * Reinitialize FE data for a lower dimenesional element with a given set of reference points
   */
  void reinitLowerDElem(const Elem * elem,
                        const std::vector<Point> * const pts = nullptr,
                        const std::vector<Real> * const weights = nullptr);

  /**
   * reinitialize a neighboring lower dimensional element
   */
  void reinitNeighborLowerDElem(const Elem * elem);

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
  const QBase * const & qRuleMortar() const { return constify_ref(_qrule_msm); }

private:
  /**
   * compute AD things on an element face
   */
  void computeADFace(const Elem & elem, const unsigned int side);

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

  void reinitFVFace(const FaceInfo & fi);

  /**
   * Reinitialize an element and its neighbor along a particular side.
   *
   * @param elem Element being reinitialized
   * @param side Side of the element
   * @param neighbor Neighbor facing the element on the side 'side'
   * @param neighbor_side The side id on the neighboring element.
   * @param neighbor_reference_points Optional argument specifying the neighbor reference points. If
   * not passed, then neighbor reference points will be determined by doing an inverse map based on
   * the physical location of the \p elem quadrature points
   */
  void reinitElemAndNeighbor(const Elem * elem,
                             unsigned int side,
                             const Elem * neighbor,
                             unsigned int neighbor_side,
                             const std::vector<Point> * neighbor_reference_points = nullptr);

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
  void prepareVariable(MooseVariableFieldBase * var);
  void prepareVariableNonlocal(MooseVariableFieldBase * var);
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
  void copyShapes(MooseVariableField<T> & v);
  void copyShapes(unsigned int var);

  template <typename T>
  void copyFaceShapes(MooseVariableField<T> & v);
  void copyFaceShapes(unsigned int var);

  template <typename T>
  void copyNeighborShapes(MooseVariableField<T> & v);
  void copyNeighborShapes(unsigned int var);

  /**
   * Add local residuals of all field variables for a set of tags onto the global residual vectors
   * associated with the tags.
   */
  void addResidual(const std::vector<VectorTag> & vector_tags);
  /**
   * Add local neighbor residuals of all field variables for a set of tags onto the global residual
   * vectors associated with the tags.
   */
  void addResidualNeighbor(const std::vector<VectorTag> & vector_tags);
  /**
   * Add local neighbor residuals of all field variables for a set of tags onto the global residual
   * vectors associated with the tags.
   */
  void addResidualLower(const std::vector<VectorTag> & vector_tags);
  /**
   * Add residuals of all scalar variables for a set of tags onto the global residual vectors
   * associated with the tags.
   */
  void addResidualScalar(const std::vector<VectorTag> & vector_tags);

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
  void cacheResidual(dof_id_type dof, Real value, TagID tag_id);

  /**
   * Cache individual residual contributions.  These will ultimately get added to the residual when
   * addCachedResidual() is called.
   *
   * @param dof The degree of freedom to add the residual contribution to
   * @param value The value of the residual contribution.
   * @param tags the contribution should go to all tags
   */
  void cacheResidual(dof_id_type dof, Real value, const std::set<TagID> & tags);

  /**
   * Deperecated method. Use \p cacheResidual
   */
  void cacheResidualContribution(dof_id_type dof, Real value, TagID tag_id);

  /**
   * Deperecated method. Use \p cacheResidual
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
   * Pushes all cached residuals to the global residual vectors associated with each tag.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResiduals();

  /**
   * Clears all of the residuals in _cached_residual_rows and _cached_residual_values
   *
   * This method is designed specifically for use after calling
   * FEProblemBase::addCachedResidualDirectly() and DisplacedProblem::addCachedResidualDirectly() to
   * ensure that we don't have any extra residuals hanging around that we didn't have the vectors
   * for
   */
  void clearCachedResiduals();

  /**
   * Adds the values that have been cached by calling cacheResidual(), cacheResidualNeighbor(),
   * and/or cacheResidualLower() to a user-defined residual (that is, not necessarily the vector
   * that vector_tag points to)
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidualDirectly(NumericVector<Number> & residual, const VectorTag & vector_tag);

  /**
   * Sets local residuals of all field variables to the global residual vector for a tag.
   */
  void setResidual(NumericVector<Number> & residual, const VectorTag & vector_tag);

  /**
   * Sets local neighbor residuals of all field variables to the global residual vector for a tag.
   */
  void setResidualNeighbor(NumericVector<Number> & residual, const VectorTag & vector_tag);

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
                        std::vector<dof_id_type> & dof_indices,
                        TagID tag = 0);

  /**
   * Add element matrix for ivar rows and jvar columns to the global Jacobian matrix for given
   * tags.
   */
  void addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                            unsigned int ivar,
                            unsigned int jvar,
                            const DofMap & dof_map,
                            std::vector<dof_id_type> & dof_indices,
                            const std::set<TagID> & tags);

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
   * Add *all* portions of the Jacobian except PrimaryPrimary, e.g. LowerLower, LowerSecondary,
   * LowerPrimary, SecondaryLower, SecondarySecondary, SecondaryPrimary, PrimaryLower,
   * PrimarySecondary, for mortar-like objects. Primary indicates the interior parent element on the
   * primary side of the mortar interface. Secondary indicates the neighbor of the interior parent
   * element. Lower denotes the lower-dimensional element living on the primary side of the mortar
   * interface.
   */
  void addJacobianNeighborLowerD();

  /**
   * Add portions of the Jacobian of LowerLower, LowerSecondary, and SecondaryLower for
   * boundary conditions. Secondary indicates the boundary element. Lower denotes the
   * lower-dimensional element living on the boundary side.
   */
  void addJacobianLowerD();

  /**
   * Cache *all* portions of the Jacobian, e.g. LowerLower, LowerSecondary, LowerPrimary,
   * SecondaryLower, SecondarySecondary, SecondaryPrimary, PrimaryLower, PrimarySecondary,
   * PrimaryPrimary for mortar-like objects. Primary indicates the interior parent element on the
   * primary side of the mortar interface. Secondary indicates the interior parent element on the
   * secondary side of the interface. Lower denotes the lower-dimensional element living on the
   * secondary side of the mortar interface; it's the boundary face of the \p Secondary element.
   */
  void cacheJacobianMortar();

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
    jacobianBlockUsed(tag, ivar, jvar, true);
    return _sub_Kee[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Get local Jacobian block from non-local contribution for a pair of variables and a tag.
   */
  DenseMatrix<Number> & jacobianBlockNonlocal(unsigned int ivar, unsigned int jvar, TagID tag = 0)
  {
    jacobianBlockNonlocalUsed(tag, ivar, jvar, true);
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
   * Returns the jacobian block for the given mortar Jacobian type. This jacobian block can involve
   * degrees of freedom from the secondary side interior parent, the primary side
   * interior parent, or the lower-dimensional element (located on the secondary
   * side)
   */
  DenseMatrix<Number> & jacobianBlockMortar(Moose::ConstraintJacobianType type,
                                            unsigned int ivar,
                                            unsigned int jvar,
                                            TagID tag = 0);

  void cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                          const std::vector<dof_id_type> & idof_indices,
                          const std::vector<dof_id_type> & jdof_indices,
                          Real scaling_factor,
                          TagID tag = 0);

  std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> & couplingEntries()
  {
    return _cm_ff_entry;
  }
  const std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> &
  couplingEntries() const
  {
    return _cm_ff_entry;
  }
  std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> &
  nonlocalCouplingEntries()
  {
    return _cm_nonlocal_entry;
  }
  const std::vector<std::pair<MooseVariableFieldBase *, MooseVariableScalar *>> &
  fieldScalarCouplingEntries() const
  {
    return _cm_fs_entry;
  }
  const std::vector<std::pair<MooseVariableScalar *, MooseVariableFieldBase *>> &
  scalarFieldCouplingEntries() const
  {
    return _cm_sf_entry;
  }

  // Read-only references
  const VariablePhiValue & phi() const { return _phi; }
  template <typename T>
  const ADTemplateVariablePhiGradient<T> & adGradPhi(const MooseVariableFE<T> & v) const
  {
    return _ad_grad_phi_data.at(v.feType());
  }
  const VariablePhiValue & phi(const MooseVariableField<Real> &) const { return _phi; }
  const VariablePhiGradient & gradPhi() const { return _grad_phi; }
  const VariablePhiGradient & gradPhi(const MooseVariableField<Real> &) const { return _grad_phi; }
  const VariablePhiSecond & secondPhi() const { return _second_phi; }
  const VariablePhiSecond & secondPhi(const MooseVariableField<Real> &) const
  {
    return _second_phi;
  }

  const VariablePhiValue & phiFace() const { return _phi_face; }
  const VariablePhiValue & phiFace(const MooseVariableField<Real> &) const { return _phi_face; }
  const VariablePhiGradient & gradPhiFace() const { return _grad_phi_face; }
  const VariablePhiGradient & gradPhiFace(const MooseVariableField<Real> &) const
  {
    return _grad_phi_face;
  }
  const VariablePhiSecond & secondPhiFace(const MooseVariableField<Real> &) const
  {
    return _second_phi_face;
  }

  const VariablePhiValue & phiNeighbor(const MooseVariableField<Real> &) const
  {
    return _phi_neighbor;
  }
  const VariablePhiGradient & gradPhiNeighbor(const MooseVariableField<Real> &) const
  {
    return _grad_phi_neighbor;
  }
  const VariablePhiSecond & secondPhiNeighbor(const MooseVariableField<Real> &) const
  {
    return _second_phi_neighbor;
  }

  const VariablePhiValue & phiFaceNeighbor(const MooseVariableField<Real> &) const
  {
    return _phi_face_neighbor;
  }
  const VariablePhiGradient & gradPhiFaceNeighbor(const MooseVariableField<Real> &) const
  {
    return _grad_phi_face_neighbor;
  }
  const VariablePhiSecond & secondPhiFaceNeighbor(const MooseVariableField<Real> &) const
  {
    return _second_phi_face_neighbor;
  }

  const VectorVariablePhiValue & phi(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_phi;
  }
  const VectorVariablePhiGradient & gradPhi(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_grad_phi;
  }
  const VectorVariablePhiSecond & secondPhi(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_second_phi;
  }
  const VectorVariablePhiCurl & curlPhi(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_curl_phi;
  }

  const VectorVariablePhiValue & phiFace(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_phi_face;
  }
  const VectorVariablePhiGradient & gradPhiFace(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_grad_phi_face;
  }
  const VectorVariablePhiSecond & secondPhiFace(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_second_phi_face;
  }
  const VectorVariablePhiCurl & curlPhiFace(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_curl_phi_face;
  }

  const VectorVariablePhiValue & phiNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_phi_neighbor;
  }
  const VectorVariablePhiGradient &
  gradPhiNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_grad_phi_neighbor;
  }
  const VectorVariablePhiSecond &
  secondPhiNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_second_phi_neighbor;
  }
  const VectorVariablePhiCurl & curlPhiNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_curl_phi_neighbor;
  }

  const VectorVariablePhiValue & phiFaceNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_phi_face_neighbor;
  }
  const VectorVariablePhiGradient &
  gradPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_grad_phi_face_neighbor;
  }
  const VectorVariablePhiSecond &
  secondPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_second_phi_face_neighbor;
  }
  const VectorVariablePhiCurl &
  curlPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &) const
  {
    return _vector_curl_phi_face_neighbor;
  }

  // Writeable references
  VariablePhiValue & phi(const MooseVariableField<Real> &) { return _phi; }
  VariablePhiGradient & gradPhi(const MooseVariableField<Real> &) { return _grad_phi; }
  VariablePhiSecond & secondPhi(const MooseVariableField<Real> &) { return _second_phi; }

  VariablePhiValue & phiFace(const MooseVariableField<Real> &) { return _phi_face; }
  VariablePhiGradient & gradPhiFace(const MooseVariableField<Real> &) { return _grad_phi_face; }
  VariablePhiSecond & secondPhiFace(const MooseVariableField<Real> &) { return _second_phi_face; }

  VariablePhiValue & phiNeighbor(const MooseVariableField<Real> &) { return _phi_neighbor; }
  VariablePhiGradient & gradPhiNeighbor(const MooseVariableField<Real> &)
  {
    return _grad_phi_neighbor;
  }
  VariablePhiSecond & secondPhiNeighbor(const MooseVariableField<Real> &)
  {
    return _second_phi_neighbor;
  }

  VariablePhiValue & phiFaceNeighbor(const MooseVariableField<Real> &)
  {
    return _phi_face_neighbor;
  }
  VariablePhiGradient & gradPhiFaceNeighbor(const MooseVariableField<Real> &)
  {
    return _grad_phi_face_neighbor;
  }
  VariablePhiSecond & secondPhiFaceNeighbor(const MooseVariableField<Real> &)
  {
    return _second_phi_face_neighbor;
  }

  // Writeable references with vector variable
  VectorVariablePhiValue & phi(const MooseVariableField<RealVectorValue> &) { return _vector_phi; }
  VectorVariablePhiGradient & gradPhi(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_grad_phi;
  }
  VectorVariablePhiSecond & secondPhi(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_second_phi;
  }
  VectorVariablePhiCurl & curlPhi(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_curl_phi;
  }

  VectorVariablePhiValue & phiFace(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_phi_face;
  }
  VectorVariablePhiGradient & gradPhiFace(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_grad_phi_face;
  }
  VectorVariablePhiSecond & secondPhiFace(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_second_phi_face;
  }
  VectorVariablePhiCurl & curlPhiFace(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_curl_phi_face;
  }

  VectorVariablePhiValue & phiNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_phi_neighbor;
  }
  VectorVariablePhiGradient & gradPhiNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_grad_phi_neighbor;
  }
  VectorVariablePhiSecond & secondPhiNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_second_phi_neighbor;
  }
  VectorVariablePhiCurl & curlPhiNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_curl_phi_neighbor;
  }
  VectorVariablePhiValue & phiFaceNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_phi_face_neighbor;
  }
  VectorVariablePhiGradient & gradPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_grad_phi_face_neighbor;
  }
  VectorVariablePhiSecond & secondPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_second_phi_face_neighbor;
  }
  VectorVariablePhiCurl & curlPhiFaceNeighbor(const MooseVariableField<RealVectorValue> &)
  {
    return _vector_curl_phi_face_neighbor;
  }

  // Writeable references with array variable
  VariablePhiValue & phi(const MooseVariableField<RealEigenVector> &) { return _phi; }
  VariablePhiGradient & gradPhi(const MooseVariableField<RealEigenVector> &) { return _grad_phi; }
  VariablePhiSecond & secondPhi(const MooseVariableField<RealEigenVector> &) { return _second_phi; }

  VariablePhiValue & phiFace(const MooseVariableField<RealEigenVector> &) { return _phi_face; }
  VariablePhiGradient & gradPhiFace(const MooseVariableField<RealEigenVector> &)
  {
    return _grad_phi_face;
  }
  VariablePhiSecond & secondPhiFace(const MooseVariableField<RealEigenVector> &)
  {
    return _second_phi_face;
  }

  VariablePhiValue & phiNeighbor(const MooseVariableField<RealEigenVector> &)
  {
    return _phi_neighbor;
  }
  VariablePhiGradient & gradPhiNeighbor(const MooseVariableField<RealEigenVector> &)
  {
    return _grad_phi_neighbor;
  }
  VariablePhiSecond & secondPhiNeighbor(const MooseVariableField<RealEigenVector> &)
  {
    return _second_phi_neighbor;
  }

  VariablePhiValue & phiFaceNeighbor(const MooseVariableField<RealEigenVector> &)
  {
    return _phi_face_neighbor;
  }
  VariablePhiGradient & gradPhiFaceNeighbor(const MooseVariableField<RealEigenVector> &)
  {
    return _grad_phi_face_neighbor;
  }
  VariablePhiSecond & secondPhiFaceNeighbor(const MooseVariableField<RealEigenVector> &)
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
  const ADTemplateVariablePhiGradient<OutputType> & feADGradPhi(FEType type) const
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
  const typename OutputTools<OutputType>::VariablePhiValue & feDualPhiLower(FEType type) const;

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiLower(FEType type) const;

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient &
  feGradDualPhiLower(FEType type) const;

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
  const ADTemplateVariablePhiGradient<OutputType> & feADGradPhiFace(FEType type) const
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
  void cacheJacobian(numeric_index_type i, numeric_index_type j, Real value, TagID tag = 0);

  /**
   * Caches the Jacobian entry 'value', to eventually be
   * added/set in the (i,j) location of the matrices in corresponding to \p tags.
   *
   * We use numeric_index_type for the index arrays (rather than
   * dof_id_type) since that is what the SparseMatrix interface uses,
   * but at the time of this writing, those two types are equivalent.
   */
  void cacheJacobian(numeric_index_type i,
                     numeric_index_type j,
                     Real value,
                     const std::set<TagID> & tags);

  /**
   * Deprecated method. Use cacheJacobian instead
   */
  void
  cacheJacobianContribution(numeric_index_type i, numeric_index_type j, Real value, TagID tag = 0);

  /**
   * Deprecated method. Use cacheJacobian instead
   */
  void cacheJacobianContribution(numeric_index_type i,
                                 numeric_index_type j,
                                 Real value,
                                 const std::set<TagID> & tags);

  /**
   * Sets previously-cached Jacobian values via SparseMatrix::set() calls.
   */
  void setCachedJacobian();

  /**
   * Deprecated. Use \p setCachedJacobian instead
   */
  void setCachedJacobianContributions();

  /**
   * Zero out previously-cached Jacobian rows.
   */
  void zeroCachedJacobian();

  /**
   * Deprecated. Use \p zeroCachedJacobian instead
   */
  void zeroCachedJacobianContributions();

  /**
   * Deprecated. Call \p addCachedJacobian
   */
  void addCachedJacobianContributions();

  /// On-demand computation of volume element accounting for RZ/RSpherical
  Real elementVolume(const Elem * elem) const;

  /**
   * Set the pointer to the XFEM controller object
   */
  void setXFEM(std::shared_ptr<XFEMInterface> xfem) { _xfem = xfem; }

  /**
   * Assign the displacement numbers and directions
   */
  void assignDisplacements(
      std::vector<std::pair<unsigned int, unsigned short>> && disp_numbers_and_directions);

  /**
   * Helper function for assembling residual contriubutions on local
   * quadrature points for an array kernel, bc, etc.
   * @param re The local residual
   * @param i The local test function index
   * @param ntest The number of test functions
   * @param v The residual contribution on the current qp
   */
  void saveLocalArrayResidual(DenseVector<Number> & re,
                              unsigned int i,
                              unsigned int ntest,
                              const RealEigenVector & v) const
  {
    for (unsigned int j = 0; j < v.size(); ++j, i += ntest)
      re(i) += v(j);
  }

  /**
   * Helper function for assembling diagonal Jacobian contriubutions on local
   * quadrature points for an array kernel, bc, etc.
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
                                  const RealEigenVector & v) const
  {
    unsigned int pace = (_component_block_diagonal[ivar] ? 0 : nphi);
    for (unsigned int k = 0; k < v.size(); ++k, i += ntest, j += pace)
      ke(i, j) += v(k);
  }

  /**
   * Helper function for assembling full Jacobian contriubutions on local
   * quadrature points for an array kernel, bc, etc.
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
                                  const RealEigenMatrix & v) const
  {
    if (ivar == jvar && _component_block_diagonal[ivar])
    {
      for (unsigned int k = 0; k < v.rows(); ++k, i += ntest)
        ke(i, j) += v(k, k);
    }
    else
    {
      const unsigned int saved_j = j;
      for (unsigned int k = 0; k < v.rows(); ++k, i += ntest)
      {
        j = saved_j;
        for (unsigned int l = 0; l < v.cols(); ++l, j += nphi)
          ke(i, j) += v(k, l);
      }
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

  /**
   * Attaches the current elem/volume quadrature rule to the given fe.  The
   * current subdomain (as set via setCurrentSubdomainID is used to determine
   * the correct rule.  The attached quadrature rule is also returned.
   */
  inline const QBase * attachQRuleElem(unsigned int dim, FEBase & fe)
  {
    auto qrule = qrules(dim).vol.get();
    fe.attach_quadrature_rule(qrule);
    return qrule;
  }

  /**
   * Attaches the current face/area quadrature rule to the given fe.  The
   * current subdomain (as set via setCurrentSubdomainID is used to determine
   * the correct rule.  The attached quadrature rule is also returned.
   */
  inline const QBase * attachQRuleFace(unsigned int dim, FEBase & fe)
  {
    auto qrule = qrules(dim).face.get();
    fe.attach_quadrature_rule(qrule);
    return qrule;
  }

  /**
   * This simply caches the residual value for the corresponding index for the provided
   * \p vector_tags, and applies any scaling factors. The scaling factor is defined in
   * _scaling_vector if global AD indexing is used. Otherwise, a uniform scaling factor of 1.0 is
   * used.
   */
  void processResidual(Real residual, dof_id_type dof_index, const std::set<TagID> & vector_tags);

  /**
   * This simply caches the derivative values for the corresponding column indices for the provided
   * \p matrix_tags, and applies any scaling factors
   */
  void processJacobian(const ADReal & residual,
                       dof_id_type dof_index,
                       const std::set<TagID> & matrix_tags);

  /**
   * This simply caches the derivative values for the corresponding column indices for the provided
   * \p matrix_tags, without applying any scaling factors
   */
  void processJacobianNoScaling(const ADReal & residual,
                                dof_id_type dof_index,
                                const std::set<TagID> & matrix_tags);

  /**
   * This performs the duties of both \p processResidual and \p processJacobian
   */
  void processResidualAndJacobian(const ADReal & residual,
                                  dof_id_type dof_index,
                                  const std::set<TagID> & vector_tags,
                                  const std::set<TagID> & matrix_tags);

  /**
   * Process the \p derivatives() data of an \p ADReal. When using global indexing, this method
   * simply caches the derivative values for the corresponding column indices for the provided
   * \p matrix_tags. Note that this single dof overload will not call \p
   * DofMap::constraint_element_matrix.
   *
   * If not using global indexing, then the user must provide a
   * functor which takes three arguments: the <tt>ADReal residual</tt> that contains the derivatives
   * to be processed, the \p row_index corresponding to the row index of the matrices that values
   * should be added to, and the \p matrix_tags specifying the matrices that will  be added into
   */
  template <typename LocalFunctor>
  void processJacobian(const ADReal & residual,
                       dof_id_type dof_index,
                       const std::set<TagID> & matrix_tags,
                       LocalFunctor & local_functor);

  /**
   * Process the supplied residual values. This is a mirror of of the non-templated version of \p
   * processResiduals except that it's meant for \emph only processing residuals (and not their
   * derivatives/Jacobian). We supply this API such that residual objects that leverage the AD
   * version of this method when computing the Jacobian (or residual + Jacobian) can mirror the same
   * behavior when doing pure residual evaluations, such as when evaluting linear residuals during
   * (P)JFNK. This method will call \p constrain_element_vector on the supplied residuals
   */
  template <typename T>
  void processResiduals(const std::vector<T> & residuals,
                        const std::vector<dof_id_type> & row_indices,
                        const std::set<TagID> & vector_tags,
                        Real scaling_factor);

  /**
   * Process the value and \p derivatives() data of a vector of \p ADReals. When using global
   * indexing, this method simply caches the value (residual) for the provided \p vector_tags and
   * derivative values (Jacobian) for the corresponding column indices for the provided \p
   * matrix_tags. Note that this overload will call \p DofMap::constrain_element_vector and \p
   * DofMap::constrain_element_matrix
   */
  void processResidualsAndJacobian(const std::vector<ADReal> & residuals,
                                   const std::vector<dof_id_type> & row_indices,
                                   const std::set<TagID> & vector_tags,
                                   const std::set<TagID> & matrix_tags,
                                   Real scaling_factor);

  /**
   * Process the \p derivatives() data of a vector of \p ADReals. When using global indexing, this
   * method simply caches the derivative values for the corresponding column indices for the
   * provided \p matrix_tags. Note that this overload will call \p DofMap::constrain_element_matrix.
   *
   * If not using global indexing, then the user must provide a functor which takes three arguments:
   * the <tt>std::vector<ADReal> residuals</tt> that contains the derivatives to be processed, the
   * <tt>std::vector<dof_id_type>row_indices</tt> corresponding to the row indices of the matrices
   * that values should be added to, and the \p matrix_tags specifying the matrices that will be
   * added into
   */
  template <typename LocalFunctor>
  void processJacobian(const std::vector<ADReal> & residuals,
                       const std::vector<dof_id_type> & row_indices,
                       const std::set<TagID> & matrix_tags,
                       Real scaling_factor,
                       LocalFunctor & local_functor);

  /**
   * Same as \p processResiduals with the exception that constrain_element_vector and
   * constrain_element_matrix will not be applied. This should only be used when the contributions
   * of these residuals to libmesh constrained degrees of freedom should be 0, e.g. if the residuals
   * correspond to mortar constraint residuals along faces such that interior hanging nodes will not
   * feel the contribution
   */
  void processUnconstrainedResidualsAndJacobian(const std::vector<ADReal> & residuals,
                                                const std::vector<dof_id_type> & row_indices,
                                                const std::set<TagID> & vector_tags,
                                                const std::set<TagID> & matrix_tags,
                                                Real scaling_factor);

  /**
   * signals this object that a vector containing variable scaling factors should be used when
   * doing residual and matrix assembly
   */
  void hasScalingVector();

  /**
   * Modify the weights when using the arbitrary quadrature rule. The intention is to use this when
   * you wish to supply your own quadrature after calling reinit at physical points.
   *
   * You should only use this if the arbitrary quadrature is the current quadrature rule!
   *
   * @param weights The weights to fill into _current_JxW
   */
  void modifyArbitraryWeights(const std::vector<Real> & weights);

  /**
   * @return whether we are computing a residual
   */
  bool computingResidual() const { return _computing_residual; }

  /**
   * @return whether we are computing a Jacobian
   */
  bool computingJacobian() const { return _computing_jacobian; }

  /**
   * @return whether we are computing a residual and a Jacobian simultaneously
   */
  bool computingResidualAndJacobian() const { return _computing_residual_and_jacobian; }

  /**
   * @return The current mortar segment element
   */
  const Elem * const & msmElem() const { return _msm_elem; }

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

  void computeFaceMap(const Elem & elem, const unsigned int side, const std::vector<Real> & qw);

  void reinitFEFaceNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  void reinitFENeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  template <typename Points, typename Coords>
  void setCoordinateTransformation(const QBase * qrule,
                                   const Points & q_points,
                                   Coords & coord,
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
   * Clear any currently cached jacobians
   *
   * This is automatically called by setCachedJacobian
   */
  void clearCachedJacobian();

  /**
   * Deprecated. Call \p clearCachedJacobian
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

  /**
   * compute gradient of phi possibly with derivative information with respect to nonlinear
   * displacement variables
   */
  template <typename OutputType>
  void computeGradPhiAD(const Elem * elem,
                        unsigned int n_qp,
                        ADTemplateVariablePhiGradient<OutputType> & grad_phi,
                        FEGenericBase<OutputType> * fe);

  /**
   * resize any objects that contribute to automatic differentiation-related mapping calculations
   */
  void resizeADMappingObjects(unsigned int n_qp, unsigned int dim);

  /**
   * compute the finite element reference-physical mapping quantities (such as JxW) with possible
   * dependence on nonlinear displacement variables at a single quadrature point
   */
  void
  computeSinglePointMapAD(const Elem * elem, const std::vector<Real> & qw, unsigned p, FEBase * fe);

  /**
   * Add local residuals of all field variables for a tag onto the tag's residual vector
   */
  void addResidual(const VectorTag & vector_tag);
  /**
   * Add local neighbor residuals of all field variables for a tag onto the tag's residual vector
   */
  void addResidualNeighbor(const VectorTag & vector_tag);
  /**
   * Add local neighbor residuals of all field variables for a tag onto the tag's residual vector
   */
  void addResidualLower(const VectorTag & vector_tag);
  /**
   * Add residuals of all scalar variables for a tag onto the tag's residual vector
   */
  void addResidualScalar(const VectorTag & vector_tag);

  /**
   * Clears all of the residuals for a specific vector tag
   */
  void clearCachedResiduals(const VectorTag & vector_tag);

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

  void buildLowerDDualFE(FEType type) const;

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
  void buildVectorDualLowerDFE(FEType type) const;

  /**
   * Sets whether or not Jacobian coupling between \p ivar and \p jvar is used
   * to the value \p used
   */
  void jacobianBlockUsed(TagID tag, unsigned int ivar, unsigned int jvar, bool used)
  {
    _jacobian_block_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar] = used;
  }

  /**
   * Return a flag to indicate if a particular coupling Jacobian block
   * between \p ivar and \p jvar is used
   */
  char jacobianBlockUsed(TagID tag, unsigned int ivar, unsigned int jvar) const
  {
    return _jacobian_block_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Sets whether or not neighbor Jacobian coupling between \p ivar and \p jvar is used
   * to the value \p used
   */
  void jacobianBlockNeighborUsed(TagID tag, unsigned int ivar, unsigned int jvar, bool used)
  {
    _jacobian_block_neighbor_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar] = used;
  }

  /**
   * Return a flag to indicate if a particular coupling neighbor Jacobian block
   * between \p ivar and \p jvar is used
   */
  char jacobianBlockNeighborUsed(TagID tag, unsigned int ivar, unsigned int jvar) const
  {
    return _jacobian_block_neighbor_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Sets whether or not lower Jacobian coupling between \p ivar and \p jvar is used
   * to the value \p used
   */
  void jacobianBlockLowerUsed(TagID tag, unsigned int ivar, unsigned int jvar, bool used)
  {
    _jacobian_block_lower_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar] = used;
  }

  /**
   * Return a flag to indicate if a particular coupling lower Jacobian block
   * between \p ivar and \p jvar is used
   */
  char jacobianBlockLowerUsed(TagID tag, unsigned int ivar, unsigned int jvar) const
  {
    return _jacobian_block_lower_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * Sets whether or not nonlocal Jacobian coupling between \p ivar and \p jvar is used
   * to the value \p used
   */
  void jacobianBlockNonlocalUsed(TagID tag, unsigned int ivar, unsigned int jvar, bool used)
  {
    _jacobian_block_nonlocal_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar] = used;
  }

  /**
   * Return a flag to indicate if a particular coupling nonlocal Jacobian block
   * between \p ivar and \p jvar is used
   */
  char jacobianBlockNonlocalUsed(TagID tag, unsigned int ivar, unsigned int jvar) const
  {
    return _jacobian_block_nonlocal_used[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
  }

  /**
   * This simply caches the derivative values for the corresponding column indices for the provided
   * \p matrix_tags, and applies the supplied scaling factor
   */
  void processJacobian(const ADReal & residual,
                       dof_id_type dof_index,
                       const std::set<TagID> & matrix_tags,
                       Real scaling_factor);

  SystemBase & _sys;
  SubProblem & _subproblem;

  const bool _displaced;

  /// Coupling matrices
  const CouplingMatrix * _cm;
  const CouplingMatrix & _nonlocal_cm;

  /// Whether we are currently computing the residual
  const bool & _computing_residual;

  /// Whether we are currently computing the Jacobian
  const bool & _computing_jacobian;

  /// Whether we are currently computing the residual and Jacobian
  const bool & _computing_residual_and_jacobian;

  /// Entries in the coupling matrix for field variables
  std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> _cm_ff_entry;
  /// Entries in the coupling matrix for field variables vs scalar variables
  std::vector<std::pair<MooseVariableFieldBase *, MooseVariableScalar *>> _cm_fs_entry;
  /// Entries in the coupling matrix for scalar variables vs field variables
  std::vector<std::pair<MooseVariableScalar *, MooseVariableFieldBase *>> _cm_sf_entry;
  /// Entries in the coupling matrix for scalar variables
  std::vector<std::pair<MooseVariableScalar *, MooseVariableScalar *>> _cm_ss_entry;
  /// Entries in the coupling matrix for field variables for nonlocal calculations
  std::vector<std::pair<MooseVariableFieldBase *, MooseVariableFieldBase *>> _cm_nonlocal_entry;
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
  /// Each dimension's actual vector fe objects indexed on type
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe;
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

  /// Data structure for tracking/grouping a set of quadrature rules for a
  /// particular dimensionality of mesh element.
  struct QRules
  {
    QRules()
      : vol(nullptr),
        face(nullptr),
        arbitrary_vol(nullptr),
        arbitrary_face(nullptr),
        neighbor(nullptr)
    {
    }

    /// volume/elem (meshdim) quadrature rule
    std::unique_ptr<QBase> vol;
    /// area/face (meshdim-1) quadrature rule
    std::unique_ptr<QBase> face;
    /// finite volume face/flux quadrature rule (meshdim-1)
    std::unique_ptr<QBase> fv_face;
    /// volume/elem (meshdim) custom points quadrature rule
    std::unique_ptr<ArbitraryQuadrature> arbitrary_vol;
    /// area/face (meshdim-1) custom points quadrature rule
    std::unique_ptr<ArbitraryQuadrature> arbitrary_face;
    /// area/face (meshdim-1) custom points quadrature rule for DG
    std::unique_ptr<ArbitraryQuadrature> neighbor;
  };

  /// Holds quadrature rules for each dimension.  These are created up front
  /// at the start of the simulation and reused/referenced for the remainder of
  /// the sim.  This data structure should generally be read/accessed via the
  /// qrules() function.
  std::unordered_map<SubdomainID, std::vector<QRules>> _qrules;

  /// This is an abstraction over the internal qrules function.  This is
  /// necessary for faces because (nodes of) faces can exists in more than one
  /// subdomain.  When this is the case, we need to use the quadrature rule from
  /// the subdomain that has the highest specified quadrature order.  So when
  /// you need to access a face quadrature rule, you should retrieve it via this
  /// function.
  QBase * qruleFace(const Elem * elem, unsigned int side);
  ArbitraryQuadrature * qruleArbitraryFace(const Elem * elem, unsigned int side);

  template <typename T>
  T * qruleFaceHelper(const Elem * elem, unsigned int side, std::function<T *(QRules &)> rule_fn)
  {
    auto dim = elem->dim();
    auto neighbor = elem->neighbor_ptr(side);
    auto q = rule_fn(qrules(dim, elem->subdomain_id()));
    if (!neighbor)
      return q;

    // find the maximum face quadrature order for all blocks the face is in
    auto neighbor_block = neighbor->subdomain_id();
    if (neighbor_block == elem->subdomain_id())
      return q;

    auto q_neighbor = rule_fn(qrules(dim, neighbor_block));
    if (q->get_order() > q_neighbor->get_order())
      return q;
    return q_neighbor;
  }

  inline QRules & qrules(unsigned int dim) { return qrules(dim, _current_subdomain_id); }

  /// This is a helper function for accessing quadrature rules for a
  /// particular dimensionality of element.  All access to quadrature rules in
  /// Assembly should be done via this accessor function.
  inline QRules & qrules(unsigned int dim, SubdomainID block)
  {
    if (_qrules.find(block) == _qrules.end())
    {
      mooseAssert(_qrules.find(Moose::ANY_BLOCK_ID) != _qrules.end(),
                  "missing quadrature rules for specified block");
      mooseAssert(_qrules[Moose::ANY_BLOCK_ID].size() > dim,
                  "quadrature rules not sized property for dimension");
      return _qrules[Moose::ANY_BLOCK_ID][dim];
    }
    mooseAssert(_qrules.find(block) != _qrules.end(),
                "missing quadrature rules for specified block");
    mooseAssert(_qrules[block].size() > dim, "quadrature rules not sized property for dimension");
    return _qrules[block][dim];
  }

  /**** Face Stuff ****/

  /// types of finite elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face;
  /// types of vector finite elements
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face;
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
  /// The current neighbor Normal vectors at the quadrature points.
  MooseArray<Point> _current_neighbor_normals;
  /// Mapped normals
  std::vector<Eigen::Map<RealDIMValue>> _mapped_normals;
  /// The current tangent vectors at the quadrature points
  MooseArray<std::vector<Point>> _current_tangents;

  /// Extra element IDs
  std::vector<dof_id_type> _extra_elem_ids;
  /// Extra element IDs of neighbor
  std::vector<dof_id_type> _neighbor_extra_elem_ids;
  /// Holds pointers to the dimension's normal vectors
  std::map<unsigned int, const std::vector<Point> *> _holder_normals;

  /**** Neighbor Stuff ****/

  /// types of finite elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_neighbor;
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face_neighbor;

  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_neighbor_helper;
  std::map<unsigned int, FEBase **> _holder_fe_face_neighbor_helper;

  /// FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, FEBase *>> _fe_lower;
  /// Vector FE objects for lower dimensional elements
  mutable std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_lower;
  /// helper object for transforming coordinates for lower dimensional element quadrature points
  std::map<unsigned int, FEBase **> _holder_fe_lower_helper;

  /// quadrature rule used on neighbors
  QBase * _current_qrule_neighbor;
  /// The current quadrature points on the neighbor face
  MooseArray<Point> _current_q_points_face_neighbor;
  /// Flag to indicate that JxW_neighbor is needed
  mutable bool _need_JxW_neighbor;
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
  /// Flag specifying whether a custom quadrature rule has been specified for mortar segment mesh
  bool _custom_mortar_qrule;

private:
  /// quadrature rule used on lower dimensional elements. This should always be
  /// the same as the face qrule
  QBase * _current_qrule_lower;

protected:
  /// The current "element" we are currently on.
  const Elem * _current_elem;
  /// The current subdomain ID
  SubdomainID _current_subdomain_id;
  /// The current boundary ID
  BoundaryID _current_boundary_id;
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
  /// The current neighboring lower dimensional element
  const Elem * _current_neighbor_lower_d_elem;
  /// Whether we need to compute the lower dimensional element volume
  mutable bool _need_lower_d_elem_volume;
  /// The current lower dimensional element volume
  Real _current_lower_d_elem_volume;
  /// Whether we need to compute the neighboring lower dimensional element volume
  mutable bool _need_neighbor_lower_d_elem_volume;
  /// The current neighboring lower dimensional element volume
  Real _current_neighbor_lower_d_elem_volume;
  /// Whether dual shape functions need to be computed for mortar constraints
  bool _need_dual;

  /// This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
  MooseArray<Point> _current_physical_points;

  /*
   * Residual contributions <tag_index, ivar>
   *
   * tag_index is the index into _residual_vector_tags, that is, _sub_Re[0] corresponds to the tag
   * with TagID _residual_vector_tags[0]._id
   *
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
  /// dlower/dsecondary (or dlower/delement)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kle;
  /// dlower/dprimary (or dlower/dneighbor)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kln;
  /// dsecondary/dlower (or delement/dlower)
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kel;
  /// dprimary/dlower (or dneighbor/dlower)
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
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data;
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data_face;
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data_neighbor;
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data_face_neighbor;
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data_lower;
  mutable std::map<FEType, std::unique_ptr<FEShapeData>> _fe_shape_data_dual_lower;

  /// Shape function values, gradients, second derivatives for each vector FE type
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data;
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data_face;
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data_neighbor;
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data_face_neighbor;
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data_lower;
  mutable std::map<FEType, std::unique_ptr<VectorFEShapeData>> _vector_fe_shape_data_dual_lower;

  mutable std::map<FEType, ADTemplateVariablePhiGradient<Real>> _ad_grad_phi_data;
  mutable std::map<FEType, ADTemplateVariablePhiGradient<RealVectorValue>> _ad_vector_grad_phi_data;
  mutable std::map<FEType, ADTemplateVariablePhiGradient<Real>> _ad_grad_phi_data_face;
  mutable std::map<FEType, ADTemplateVariablePhiGradient<RealVectorValue>>
      _ad_vector_grad_phi_data_face;

  /**
   * The residual vector tags that Assembly could possibly contribute to.
   *
   * The following variables are all indexed with this vector (i.e., index 0 in the following
   * vectors corresponds to the tag with TagID _residual_vector_tags[0]._id):
   * _sub_Re, _sub_Rn, _sub_Rl, _cached_residual_rows, _cached_residual_values,
   *
   * This index is also available in VectorTag::_type_id
   */
  const std::vector<VectorTag> & _residual_vector_tags;

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

  /**
   * Container of displacement numbers and directions
   */
  std::vector<std::pair<unsigned int, unsigned short>> _disp_numbers_and_directions;

  mutable bool _calculate_xyz;
  mutable bool _calculate_face_xyz;
  mutable bool _calculate_curvatures;

  /// Whether to calculate coord with AD. This will only be set to \p true if a consumer calls
  /// adCoordTransformation()
  mutable bool _calculate_ad_coord;

  mutable std::map<FEType, bool> _need_second_derivative;
  mutable std::map<FEType, bool> _need_second_derivative_neighbor;
  mutable std::map<FEType, bool> _need_curl;

  /// The map from global index to variable scaling factor
  const NumericVector<Real> * _scaling_vector = nullptr;

  /// In place side element builder for _current_side_elem
  ElemSideBuilder _current_side_elem_builder;
  /// In place side element builder for _current_neighbor_side_elem
  ElemSideBuilder _current_neighbor_side_elem_builder;
  /// In place side element builder for computeFaceMap()
  ElemSideBuilder _compute_face_map_side_elem_builder;

  const Elem * _msm_elem = nullptr;
};

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiValue &
Assembly::fePhiLower(FEType type) const
{
  buildLowerDFE(type);
  return _fe_shape_data_lower[type]->_phi;
}

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiValue &
Assembly::feDualPhiLower(FEType type) const
{
  buildLowerDDualFE(type);
  return _fe_shape_data_dual_lower[type]->_phi;
}

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiGradient &
Assembly::feGradPhiLower(FEType type) const
{
  buildLowerDFE(type);
  return _fe_shape_data_lower[type]->_grad_phi;
}

template <typename OutputType>
const typename OutputTools<OutputType>::VariablePhiGradient &
Assembly::feGradDualPhiLower(FEType type) const
{
  buildLowerDDualFE(type);
  return _fe_shape_data_dual_lower[type]->_grad_phi;
}

template <>
inline const ADTemplateVariablePhiGradient<RealVectorValue> &
Assembly::feADGradPhi<RealVectorValue>(FEType type) const
{
  return _ad_vector_grad_phi_data[type];
}

template <>
inline const ADTemplateVariablePhiGradient<RealVectorValue> &
Assembly::feADGradPhiFace<RealVectorValue>(FEType type) const
{
  return _ad_vector_grad_phi_data_face[type];
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
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::feDualPhiLower<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiLower<VectorValue<Real>>(FEType type) const;

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradDualPhiLower<VectorValue<Real>>(FEType type) const;

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
inline const ADTemplateVariablePhiGradient<RealVectorValue> &
Assembly::adGradPhi<RealVectorValue>(const MooseVariableFE<RealVectorValue> & v) const
{
  return _ad_vector_grad_phi_data.at(v.feType());
}

inline void
Assembly::processJacobian(const ADReal & residual,
                          const dof_id_type dof_index,
                          const std::set<TagID> & matrix_tags,
                          const Real scaling_factor)
{
  const auto & derivs = residual.derivatives();

  const auto & column_indices = derivs.nude_indices();
  const auto & values = derivs.nude_data();

  mooseAssert(column_indices.size() == values.size(), "Indices and values size must be the same");

  for (std::size_t i = 0; i < column_indices.size(); ++i)
    cacheJacobian(dof_index, column_indices[i], values[i] * scaling_factor, matrix_tags);
}

inline void
Assembly::processJacobian(const ADReal & residual,
                          const dof_id_type dof_index,
                          const std::set<TagID> & matrix_tags)
{
  const Real scalar = _scaling_vector ? (*_scaling_vector)(dof_index) : 1.;
  processJacobian(residual, dof_index, matrix_tags, scalar);
}

inline void
Assembly::processJacobianNoScaling(const ADReal & residual,
                                   const dof_id_type dof_index,
                                   const std::set<TagID> & matrix_tags)
{
  processJacobian(residual, dof_index, matrix_tags, 1);
}

template <typename LocalFunctor>
void
Assembly::processJacobian(const ADReal & residual,
                          const dof_id_type dof_index,
                          const std::set<TagID> & matrix_tags,
                          LocalFunctor &)
{
  processJacobian(residual, dof_index, matrix_tags);
}

template <typename LocalFunctor>
void
Assembly::processJacobian(const std::vector<ADReal> & residuals,
                          const std::vector<dof_id_type> & input_row_indices,
                          const std::set<TagID> & matrix_tags,
                          const Real scaling_factor,
                          LocalFunctor &)
{
  processResidualsAndJacobian(residuals, input_row_indices, {}, matrix_tags, scaling_factor);
}

template <typename T>
void
Assembly::processResiduals(const std::vector<T> & residuals,
                           const std::vector<dof_id_type> & input_row_indices,
                           const std::set<TagID> & vector_tags,
                           const Real scaling_factor)
{
  if (!computingResidual() || vector_tags.empty())
    return;

  mooseAssert(residuals.size() == input_row_indices.size(),
              "The number of residuals should match the number of dof indices");
  mooseAssert(residuals.size() >= 1, "Why you calling me with no residuals?");

  // Need to make a copy because we might modify this in constrain_element_vector
  std::vector<dof_id_type> row_indices = input_row_indices;

  DenseVector<Number> element_vector(row_indices.size());
  for (const auto i : index_range(row_indices))
    element_vector(i) = MetaPhysicL::raw_value(residuals[i]) * scaling_factor;

  // At time of writing, this method doesn't do anything with the asymmetric_constraint_rows
  // argument, but we set it to false to be consistent with processLocalResidual
  _dof_map.constrain_element_vector(
      element_vector, row_indices, /*asymmetric_constraint_rows=*/false);

  for (const auto i : index_range(row_indices))
    cacheResidual(row_indices[i], element_vector(i), vector_tags);
}

inline const Real &
Assembly::lowerDElemVolume() const
{
  _need_lower_d_elem_volume = true;
  return _current_lower_d_elem_volume;
}

inline const Real &
Assembly::neighborLowerDElemVolume() const
{
  _need_neighbor_lower_d_elem_volume = true;
  return _current_neighbor_lower_d_elem_volume;
}

inline void
Assembly::assignDisplacements(
    std::vector<std::pair<unsigned int, unsigned short>> && disp_numbers_and_directions)
{
  _disp_numbers_and_directions = std::move(disp_numbers_and_directions);
}
