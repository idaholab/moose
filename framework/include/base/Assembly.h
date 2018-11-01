//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "MooseArray.h"
#include "MooseTypes.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/tensor_tools.h"

// libMesh forward declarations
namespace libMesh
{
class DofMap;
class CouplingMatrix;
class Elem;
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
class XFEMInterface;

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
   * Build FEs for a neighbor with a type
   * @param type The type of FE
   */
  void buildNeighborFE(FEType type);

  /**
   * Build FEs for a neighbor face with a type
   * @param type The type of FE
   */
  void buildFaceNeighborFE(FEType type);

  /**
   * Build Vector FEs with a type
   * @param type The type of FE
   */
  void buildVectorFE(FEType type);

  /**
   * Build Vector FEs for a face with a type
   * @param type The type of FE
   */
  void buildVectorFaceFE(FEType type);

  /**
   * Build Vector FEs for a neighbor with a type
   * @param type The type of FE
   */
  void buildVectorNeighborFE(FEType type);

  /**
   * Build Vector FEs for a neighbor face with a type
   * @param type The type of FE
   */
  void buildVectorFaceNeighborFE(FEType type);

  /**
   * Get a reference to a pointer that will contain the current volume FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase *& getFE(FEType type, unsigned int dim)
  {
    buildFE(type);
    return _fe[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current 'neighbor' FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase *& getFENeighbor(FEType type, unsigned int dim)
  {
    buildNeighborFE(type);
    return _fe_neighbor[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current "face" FE.
   * @param type The type of FE
   * @param dim The dimension of the current face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase *& getFEFace(FEType type, unsigned int dim)
  {
    buildFaceFE(type);
    return _fe_face[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current "neighbor" FE.
   * @param type The type of FE
   * @param dim The dimension of the neighbor face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEBase *& getFEFaceNeighbor(FEType type, unsigned int dim)
  {
    buildFaceNeighborFE(type);
    return _fe_face_neighbor[dim][type];
  }

  /**
   * Get a reference to a pointer that will contain the current volume FEVector.
   * @param type The type of FEVector
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEVectorBase *& getVectorFE(FEType type, unsigned int dim)
  {
    buildVectorFE(type);
    return _vector_fe[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current 'neighbor' FE.
   * @param type The type of FE
   * @param dim The dimension of the current volume
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEVectorBase *& getVectorFENeighbor(FEType type, unsigned int dim)
  {
    buildVectorNeighborFE(type);
    return _vector_fe_neighbor[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current "face" FE.
   * @param type The type of FE
   * @param dim The dimension of the current face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEVectorBase *& getVectorFEFace(FEType type, unsigned int dim)
  {
    buildVectorFaceFE(type);
    return _vector_fe_face[dim][type];
  }

  /**
   * GetVector a reference to a pointer that will contain the current "neighbor" FE.
   * @param type The type of FE
   * @param dim The dimension of the neighbor face
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  FEVectorBase *& getVectorFEFaceNeighbor(FEType type, unsigned int dim)
  {
    buildVectorFaceNeighborFE(type);
    return _vector_fe_face_neighbor[dim][type];
  }

  /**
   * Returns the reference to the current quadrature being used
   * @return A _reference_ to the pointer.  Make sure to store this as a reference!
   */
  QBase *& qRule() { return _current_qrule; }

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
  QBase *& qRuleFace() { return _current_qrule_face; }

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
  const Elem *& elem() { return _current_elem; }

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
  const Elem *& neighbor() { return _current_neighbor_elem; }

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
  const Real & neighborVolume();

  /**
   * Returns the reference to the current quadrature being used on a current neighbor
   * @return A _reference_.  Make sure to store this as a reference!
   */
  QBase *& qRuleNeighbor() { return _current_qrule_neighbor; }

  /**
   * Returns the reference to the transformed jacobian weights on a current face
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const MooseArray<Real> & JxWNeighbor() { return _current_JxW_neighbor; }

  /**
   * Returns the reference to the node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node *& node() { return _current_node; }

  /**
   * Returns the reference to the neighboring node
   * @return A _reference_.  Make sure to store this as a reference!
   */
  const Node *& nodeNeighbor() { return _current_neighbor_node; }

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

  /// Deprecated init method
  void init();

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

  void addResidual(NumericVector<Number> & residual, TagID tag_id = 0);
  void addResidual(const std::map<TagName, TagID> & tags);
  void addResidualNeighbor(NumericVector<Number> & residual, TagID tag_id = 0);
  void addResidualNeighbor(const std::map<TagName, TagID> & tags);
  void addResidualScalar(TagID tag_id);
  void addResidualScalar(const std::map<TagName, TagID> & tags);

  /**
   * Takes the values that are currently in _sub_Re and appends them to the cached values.
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
                          std::vector<dof_id_type> & dof_index,
                          TagID tag = 0);

  /**
   * Takes the values that are currently in _sub_Ke and appends them to the cached values.
   */
  void cacheResidualNeighbor();

  void addCachedResiduals();

  /**
   * Adds the values that have been cached by calling cacheResidual() and or cacheResidualNeighbor()
   * to the residual.
   *
   * Note that this will also clear the cache.
   */
  void addCachedResidual(NumericVector<Number> & residual, TagID tag_id);

  void setResidual(NumericVector<Number> & residual, TagID tag_id = 0);
  void setResidualNeighbor(NumericVector<Number> & residual, TagID tag_id = 0);

  void addJacobian();
  /**
   * Adds element matrix for ivar rows and jvar columns
   */
  void addJacobianCoupledVarPair(MooseVariableBase * ivar, MooseVariableBase * jvar);
  void addJacobianNonlocal();
  void addJacobianBlock(SparseMatrix<Number> & jacobian,
                        unsigned int ivar,
                        unsigned int jvar,
                        const DofMap & dof_map,
                        std::vector<dof_id_type> & dof_indices);
  void addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                const std::vector<dof_id_type> & idof_indices,
                                const std::vector<dof_id_type> & jdof_indices);
  void addJacobianNeighbor();
  void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                           unsigned int ivar,
                           unsigned int jvar,
                           const DofMap & dof_map,
                           std::vector<dof_id_type> & dof_indices,
                           std::vector<dof_id_type> & neighbor_dof_indices);
  void addJacobianScalar();
  void addJacobianOffDiagScalar(unsigned int ivar);

  /**
   * Takes the values that are currently in _sub_Kee and appends them to the cached values.
   */
  void cacheJacobian();

  /**
   * Caches element matrix for ivar rows and jvar columns
   */
  void cacheJacobianCoupledVarPair(MooseVariableBase * ivar, MooseVariableBase * jvar);

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
  void addCachedJacobian(SparseMatrix<Number> & jacobian);

  void addCachedJacobian();

  DenseVector<Number> & residualBlock(unsigned int var_num, TagID tag_id = 0)
  {
    return _sub_Re[static_cast<unsigned int>(tag_id)][var_num];
  }

  DenseVector<Number> & residualBlockNeighbor(unsigned int var_num, TagID tag_id = 0)
  {
    return _sub_Rn[static_cast<unsigned int>(tag_id)][var_num];
  }

  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar, TagID tag = 0);

  DenseMatrix<Number> & jacobianBlockNonlocal(unsigned int ivar, unsigned int jvar, TagID tag = 0);
  DenseMatrix<Number> & jacobianBlockNeighbor(Moose::DGJacobianType type,
                                              unsigned int ivar,
                                              unsigned int jvar,
                                              TagID tag = 0);
  void cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                          std::vector<dof_id_type> & idof_indices,
                          std::vector<dof_id_type> & jdof_indices,
                          Real scaling_factor,
                          TagID tag = 0);
  void cacheJacobianBlockNonlocal(DenseMatrix<Number> & jac_block,
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
  const VariablePhiValue & phi(MooseVariable &) const { return _phi; }
  const VariablePhiGradient & gradPhi() const { return _grad_phi; }
  const VariablePhiGradient & gradPhi(MooseVariable &) const { return _grad_phi; }
  const VariablePhiSecond & secondPhi() const { return _second_phi; }
  const VariablePhiSecond & secondPhi(MooseVariable &) const { return _second_phi; }

  const VariablePhiValue & phiFace() const { return _phi_face; }
  const VariablePhiValue & phiFace(MooseVariable &) const { return _phi_face; }
  const VariablePhiGradient & gradPhiFace() const { return _grad_phi_face; }
  const VariablePhiGradient & gradPhiFace(MooseVariable &) const { return _grad_phi_face; }
  const VariablePhiSecond & secondPhiFace(MooseVariable &) const { return _second_phi_face; }

  const VariablePhiValue & phiNeighbor(MooseVariable &) const { return _phi_neighbor; }
  const VariablePhiGradient & gradPhiNeighbor(MooseVariable &) const { return _grad_phi_neighbor; }
  const VariablePhiSecond & secondPhiNeighbor(MooseVariable &) const
  {
    return _second_phi_neighbor;
  }

  const VariablePhiValue & phiFaceNeighbor(MooseVariable &) const { return _phi_face_neighbor; }
  const VariablePhiGradient & gradPhiFaceNeighbor(MooseVariable &) const
  {
    return _grad_phi_face_neighbor;
  }
  const VariablePhiSecond & secondPhiFaceNeighbor(MooseVariable &) const
  {
    return _second_phi_face_neighbor;
  }

  const VectorVariablePhiValue & phi(VectorMooseVariable &) const { return _vector_phi; }
  const VectorVariablePhiGradient & gradPhi(VectorMooseVariable &) const
  {
    return _vector_grad_phi;
  }
  const VectorVariablePhiSecond & secondPhi(VectorMooseVariable &) const
  {
    return _vector_second_phi;
  }
  const VectorVariablePhiCurl & curlPhi(VectorMooseVariable &) const { return _vector_curl_phi; }

  const VectorVariablePhiValue & phiFace(VectorMooseVariable &) const { return _vector_phi_face; }
  const VectorVariablePhiGradient & gradPhiFace(VectorMooseVariable &) const
  {
    return _vector_grad_phi_face;
  }
  const VectorVariablePhiSecond & secondPhiFace(VectorMooseVariable &) const
  {
    return _vector_second_phi_face;
  }
  const VectorVariablePhiCurl & curlPhiFace(VectorMooseVariable &) const
  {
    return _vector_curl_phi_face;
  }

  const VectorVariablePhiValue & phiNeighbor(VectorMooseVariable &) const
  {
    return _vector_phi_neighbor;
  }
  const VectorVariablePhiGradient & gradPhiNeighbor(VectorMooseVariable &) const
  {
    return _vector_grad_phi_neighbor;
  }
  const VectorVariablePhiSecond & secondPhiNeighbor(VectorMooseVariable &) const
  {
    return _vector_second_phi_neighbor;
  }
  const VectorVariablePhiCurl & curlPhiNeighbor(VectorMooseVariable &) const
  {
    return _vector_curl_phi_neighbor;
  }

  const VectorVariablePhiValue & phiFaceNeighbor(VectorMooseVariable &) const
  {
    return _vector_phi_face_neighbor;
  }
  const VectorVariablePhiGradient & gradPhiFaceNeighbor(VectorMooseVariable &) const
  {
    return _vector_grad_phi_face_neighbor;
  }
  const VectorVariablePhiSecond & secondPhiFaceNeighbor(VectorMooseVariable &) const
  {
    return _vector_second_phi_face_neighbor;
  }
  const VectorVariablePhiCurl & curlPhiFaceNeighbor(VectorMooseVariable &) const
  {
    return _vector_curl_phi_face_neighbor;
  }

  // Writeable references
  VariablePhiValue & phi(MooseVariable &) { return _phi; }
  VariablePhiGradient & gradPhi(MooseVariable &) { return _grad_phi; }
  VariablePhiSecond & secondPhi(MooseVariable &) { return _second_phi; }

  VariablePhiValue & phiFace(MooseVariable &) { return _phi_face; }
  VariablePhiGradient & gradPhiFace(MooseVariable &) { return _grad_phi_face; }
  VariablePhiSecond & secondPhiFace(MooseVariable &) { return _second_phi_face; }

  VariablePhiValue & phiNeighbor(MooseVariable &) { return _phi_neighbor; }
  VariablePhiGradient & gradPhiNeighbor(MooseVariable &) { return _grad_phi_neighbor; }
  VariablePhiSecond & secondPhiNeighbor(MooseVariable &) { return _second_phi_neighbor; }

  VariablePhiValue & phiFaceNeighbor(MooseVariable &) { return _phi_face_neighbor; }
  VariablePhiGradient & gradPhiFaceNeighbor(MooseVariable &) { return _grad_phi_face_neighbor; }
  VariablePhiSecond & secondPhiFaceNeighbor(MooseVariable &) { return _second_phi_face_neighbor; }

  VectorVariablePhiValue & phi(VectorMooseVariable &) { return _vector_phi; }
  VectorVariablePhiGradient & gradPhi(VectorMooseVariable &) { return _vector_grad_phi; }
  VectorVariablePhiSecond & secondPhi(VectorMooseVariable &) { return _vector_second_phi; }
  VectorVariablePhiCurl & curlPhi(VectorMooseVariable &) { return _vector_curl_phi; }

  VectorVariablePhiValue & phiFace(VectorMooseVariable &) { return _vector_phi_face; }
  VectorVariablePhiGradient & gradPhiFace(VectorMooseVariable &) { return _vector_grad_phi_face; }
  VectorVariablePhiSecond & secondPhiFace(VectorMooseVariable &) { return _vector_second_phi_face; }
  VectorVariablePhiCurl & curlPhiFace(VectorMooseVariable &) { return _vector_curl_phi_face; }

  VectorVariablePhiValue & phiNeighbor(VectorMooseVariable &) { return _vector_phi_neighbor; }
  VectorVariablePhiGradient & gradPhiNeighbor(VectorMooseVariable &)
  {
    return _vector_grad_phi_neighbor;
  }
  VectorVariablePhiSecond & secondPhiNeighbor(VectorMooseVariable &)
  {
    return _vector_second_phi_neighbor;
  }
  VectorVariablePhiCurl & curlPhiNeighbor(VectorMooseVariable &)
  {
    return _vector_curl_phi_neighbor;
  }

  VectorVariablePhiValue & phiFaceNeighbor(VectorMooseVariable &)
  {
    return _vector_phi_face_neighbor;
  }
  VectorVariablePhiGradient & gradPhiFaceNeighbor(VectorMooseVariable &)
  {
    return _vector_grad_phi_face_neighbor;
  }
  VectorVariablePhiSecond & secondPhiFaceNeighbor(VectorMooseVariable &)
  {
    return _vector_second_phi_face_neighbor;
  }
  VectorVariablePhiCurl & curlPhiFaceNeighbor(VectorMooseVariable &)
  {
    return _vector_curl_phi_face_neighbor;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhi(FEType type)
  {
    buildFE(type);
    return _fe_shape_data[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhi(FEType type)
  {
    buildFE(type);
    return _fe_shape_data[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhi(FEType type)
  {
    _need_second_derivative[type] = true;
    buildFE(type);
    return _fe_shape_data[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiFace(FEType type)
  {
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiFace(FEType type)
  {
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhiFace(FEType type)
  {
    _need_second_derivative[type] = true;
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiNeighbor(FEType type)
  {
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiNeighbor(FEType type)
  {
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhiNeighbor(FEType type)
  {
    _need_second_derivative_neighbor[type] = true;
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiValue & fePhiFaceNeighbor(FEType type)
  {
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiGradient & feGradPhiFaceNeighbor(FEType type)
  {
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_grad_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiSecond & feSecondPhiFaceNeighbor(FEType type)
  {
    _need_second_derivative_neighbor[type] = true;
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_second_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhi(FEType type)
  {
    _need_curl[type] = true;
    buildFE(type);
    return _fe_shape_data[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiFace(FEType type)
  {
    _need_curl[type] = true;
    buildFaceFE(type);
    return _fe_shape_data_face[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiNeighbor(FEType type)
  {
    _need_curl[type] = true;
    buildNeighborFE(type);
    return _fe_shape_data_neighbor[type]->_curl_phi;
  }

  template <typename OutputType>
  const typename OutputTools<OutputType>::VariablePhiCurl & feCurlPhiFaceNeighbor(FEType type)
  {
    _need_curl[type] = true;
    buildFaceNeighborFE(type);
    return _fe_shape_data_face_neighbor[type]->_curl_phi;
  }

  std::map<FEType, bool> _need_second_derivative;
  std::map<FEType, bool> _need_second_derivative_neighbor;
  std::map<FEType, bool> _need_curl;

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

  void reinitFEFaceNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  void reinitFENeighbor(const Elem * neighbor, const std::vector<Point> & reference_points);

  void setCoordinateTransformation(const QBase * qrule, const MooseArray<Point> & q_points);

  void computeCurrentElemVolume();

  void computeCurrentFaceVolume();

  void computeCurrentNeighborVolume();

  void addResidualBlock(NumericVector<Number> & residual,
                        DenseVector<Number> & res_block,
                        const std::vector<dof_id_type> & dof_indices,
                        Real scaling_factor);
  void cacheResidualBlock(std::vector<Real> & cached_residual_values,
                          std::vector<dof_id_type> & cached_residual_rows,
                          DenseVector<Number> & res_block,
                          std::vector<dof_id_type> & dof_indices,
                          Real scaling_factor);

  void setResidualBlock(NumericVector<Number> & residual,
                        DenseVector<Number> & res_block,
                        std::vector<dof_id_type> & dof_indices,
                        Real scaling_factor);

  void addJacobianBlock(SparseMatrix<Number> & jacobian,
                        DenseMatrix<Number> & jac_block,
                        const std::vector<dof_id_type> & idof_indices,
                        const std::vector<dof_id_type> & jdof_indices,
                        Real scaling_factor);

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

protected:
  SystemBase & _sys;

  /// Coupling matrices
  const CouplingMatrix * _cm;
  const CouplingMatrix & _nonlocal_cm;
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
  std::map<unsigned int, std::map<FEType, FEBase *>> _fe;
  /// Each dimension's actual vector fe objects indexed on type
  std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe;
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
  std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face;
  /// types of vector finite elements
  std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face;
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
  std::map<unsigned int, std::map<FEType, FEBase *>> _fe_neighbor;
  std::map<unsigned int, std::map<FEType, FEBase *>> _fe_face_neighbor;
  std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_neighbor;
  std::map<unsigned int, std::map<FEType, FEVectorBase *>> _vector_fe_face_neighbor;
  /// Each dimension's helper objects
  std::map<unsigned int, FEBase **> _holder_fe_neighbor_helper;
  std::map<unsigned int, FEBase **> _holder_fe_face_neighbor_helper;

  /// quadrature rule used on neighbors
  QBase * _current_qrule_neighbor;
  /// Holds arbitrary qrules for each dimension
  std::map<unsigned int, ArbitraryQuadrature *> _holder_qrule_neighbor;
  /// The current transformed jacobian weights on a neighbor's face
  MooseArray<Real> _current_JxW_neighbor;
  /// The current coordinate transformation coefficients
  MooseArray<Real> _coord_neighbor;

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
  bool _need_neighbor_elem_volume;
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

  /// This will be filled up with the physical points passed into reinitAtPhysical() if it is called.  Invalid at all other times.
  MooseArray<Point> _current_physical_points;

  /// residual contributions for each variable from the element
  std::vector<std::vector<DenseVector<Number>>> _sub_Re;
  /// residual contributions for each variable from the neighbor
  std::vector<std::vector<DenseVector<Number>>> _sub_Rn;
  /// auxiliary vector for scaling residuals (optimization to avoid expensive construction/destruction)
  DenseVector<Number> _tmp_Re;

  /// jacobian contributions <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kee;
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Keg;

  /// jacobian contributions from the element and neighbor <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Ken;
  /// jacobian contributions from the neighbor and element <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Kne;
  /// jacobian contributions from the neighbor <Tag, ivar, jvar>
  std::vector<std::vector<std::vector<DenseMatrix<Number>>>> _sub_Knn;

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
  std::map<FEType, FEShapeData *> _fe_shape_data;
  std::map<FEType, FEShapeData *> _fe_shape_data_face;
  std::map<FEType, FEShapeData *> _fe_shape_data_neighbor;
  std::map<FEType, FEShapeData *> _fe_shape_data_face_neighbor;

  /// Shape function values, gradients, second derivatives for each vector FE type
  std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data;
  std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_face;
  std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_neighbor;
  std::map<FEType, VectorFEShapeData *> _vector_fe_shape_data_face_neighbor;

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
  unsigned int _block_diagonal_matrix;

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
};

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhi<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhi<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhi<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFace<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFace<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFace<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFaceNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFaceNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFaceNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhi<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFace<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiNeighbor<VectorValue<Real>>(FEType type);

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFaceNeighbor<VectorValue<Real>>(FEType type);

#endif /* ASSEMBLY_H */
