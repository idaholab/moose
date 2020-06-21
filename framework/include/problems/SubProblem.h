//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Problem.h"
#include "DiracKernelInfo.h"
#include "GeometricSearchData.h"
#include "MooseTypes.h"
#include "VectorTag.h"

#include "libmesh/coupling_matrix.h"

namespace libMesh
{
template <typename>
class VectorValue;
typedef VectorValue<Real> RealVectorValue;
class GhostingFunctor;
}

class MooseMesh;
class SubProblem;
class Factory;
class Assembly;
class MooseVariableFieldBase;
class MooseVariableScalar;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;
class RestartableDataValue;
class SystemBase;
class LineSearch;
class FaceInfo;

// libMesh forward declarations
namespace libMesh
{
class EquationSystems;
class DofMap;
class CouplingMatrix;
template <typename T>
class SparseMatrix;
template <typename T>
class NumericVector;
class System;
} // namespace libMesh

template <>
InputParameters validParams<SubProblem>();

/**
 * Generic class for solving transient nonlinear problems
 *
 */
class SubProblem : public Problem
{
public:
  static InputParameters validParams();

  SubProblem(const InputParameters & parameters);
  virtual ~SubProblem();

  virtual EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;
  virtual const MooseMesh & mesh() const = 0;

  virtual bool checkNonlocalCouplingRequirement() { return _requires_nonlocal_coupling; }

  virtual void solve() = 0;
  virtual bool converged() = 0;

  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual bool isTransient() const = 0;

  /// marks this problem as including/needing finite volume functionality.
  void needFV() { _have_fv = true; }

  /// returns true if this problem includes/needs finite volume functionality.
  bool haveFV() const { return _have_fv; }

  /**
   * Whether or not the user has requested default ghosting ot be on.
   */
  bool defaultGhosting() { return _default_ghosting; }

  /**
   * Create a Tag.  Tags can be associated with Vectors and Matrices and allow objects
   * (such as Kernels) to arbitrarily contribute values to any set of vectors/matrics
   *
   * Note: If the tag is already present then this will simply return the TagID of that Tag, but the
   * type must be the same.
   *
   * @param tag_name The name of the tag to create, the TagID will get automatically generated
   * @param type The type of the tag
   */
  virtual TagID addVectorTag(const TagName & tag_name,
                             const Moose::VectorTagType type = Moose::VECTOR_TAG_RESIDUAL);

  /**
   * Get a VectorTag from a TagID.
   */
  virtual const VectorTag & getVectorTag(const TagID tag_id) const;

  /**
   * Get a TagID from a TagName.
   */
  virtual TagID getVectorTagID(const TagName & tag_name) const;

  /**
   * Retrieve the name associated with a TagID
   */
  virtual TagName vectorTagName(const TagID tag) const;

  /**
   * Return all vector tags, where a tag is represented by a map from name to ID. Can optionally be
   * limited to a vector tag type.
   */
  virtual const std::vector<VectorTag> &
  getVectorTags(const Moose::VectorTagType type = Moose::VECTOR_TAG_ANY) const;

  /**
   * Check to see if a particular Tag exists
   */
  virtual bool vectorTagExists(const TagID tag_id) const { return tag_id < _vector_tags.size(); }

  /**
   * Check to see if a particular Tag exists by using Tag name
   */
  bool vectorTagExists(const TagName & tag_name) const;

  /**
   * The total number of tags, which can be limited to the tag type
   */
  virtual unsigned int numVectorTags(const Moose::VectorTagType type = Moose::VECTOR_TAG_ANY) const;

  virtual Moose::VectorTagType vectorTagType(const TagID tag_id) const;

  /**
   * Create a Tag.  Tags can be associated with Vectors and Matrices and allow objects
   * (such as Kernels) to arbitrarily contribute values to any set of vectors/matrics
   *
   * Note: If the tag is already present then this will simply return the TagID of that Tag
   *
   * @param tag_name The name of the tag to create, the TagID will get automatically generated
   */
  virtual TagID addMatrixTag(TagName tag_name);

  /**
   * Get a TagID from a TagName.
   */
  virtual TagID getMatrixTagID(const TagName & tag_name);

  /**
   * Retrieve the name associated with a TagID
   */
  virtual TagName matrixTagName(TagID tag);

  /**
   * Check to see if a particular Tag exists
   */
  virtual bool matrixTagExists(const TagName & tag_name);

  /**
   * Check to see if a particular Tag exists
   */
  virtual bool matrixTagExists(TagID tag_id);

  /**
   * The total number of tags
   */
  virtual unsigned int numMatrixTags() const { return _matrix_tag_name_to_tag_id.size(); }

  /**
   * Return all matrix tags in the sytem, where a tag is represented by a map from name to ID
   */
  virtual std::map<TagName, TagID> & getMatrixTags() { return _matrix_tag_name_to_tag_id; }

  /// Whether or not this problem has the variable
  virtual bool hasVariable(const std::string & var_name) const = 0;

  /**
   * Returns the variable reference for requested variable which must
   * be of the expected_var_type (Nonlinear vs. Auxiliary) and
   * expected_var_field_type (standard, scalar, vector). The default
   * values of VAR_ANY and VAR_FIELD_ANY should be used when "any"
   * type of variable is acceptable.  Throws an error if the variable
   * in question is not in the expected System or of the expected
   * type.
   */
  virtual MooseVariableFieldBase &
  getVariable(THREAD_ID tid,
              const std::string & var_name,
              Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
              Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_ANY) = 0;

  /// Returns the variable reference for requested MooseVariable which may be in any system
  virtual MooseVariable & getStandardVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the variable reference for requested VectorMooseVariable which may be in any system
  virtual VectorMooseVariable & getVectorVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the variable reference for requested ArrayMooseVariable which may be in any system
  virtual ArrayMooseVariable & getArrayVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the variable name of a component of an array variable
  static std::string arrayVariableComponent(const std::string & var_name, unsigned int i)
  {
    return var_name + "_" + std::to_string(i);
  }

  /// Returns a Boolean indicating whether any system contains a variable with the name provided
  virtual bool hasScalarVariable(const std::string & var_name) const = 0;

  /// Returns the scalar variable reference from whichever system contains it
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the equation system containing the variable provided
  virtual System & getSystem(const std::string & var_name) = 0;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void
  setActiveElementalMooseVariables(const std::set<MooseVariableFieldBase *> & moose_vars,
                                   THREAD_ID tid);

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariableFieldBase *> &
  getActiveElementalMooseVariables(THREAD_ID tid) const;

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(THREAD_ID tid) const;

  /**
   * Clear the active elemental MooseVariableFieldBase.  If there are no active variables then they
   * will all be reinited. Call this after finishing the computation that was using a restricted set
   * of MooseVariableFieldBase
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Record and set the material properties required by the current computing thread.
   * @param mat_prop_ids The set of material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual void setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids,
                                           THREAD_ID tid);

  /**
   * Get the material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual const std::set<unsigned int> & getActiveMaterialProperties(THREAD_ID tid) const;

  /**
   * Method to check whether or not a list of active material roperties has been set. This method
   * is called by reinitMaterials to determine whether Material computeProperties methods need to be
   * called. If the return is False, this check prevents unnecessary material property computation
   * @param tid The thread id
   *
   * @return True if there has been a list of active material properties set, False otherwise
   */
  virtual bool hasActiveMaterialProperties(THREAD_ID tid) const;

  /**
   * Clear the active material properties. Should be called at the end of every computing thread
   *
   * @param tid The thread id
   */
  virtual void clearActiveMaterialProperties(THREAD_ID tid);

  virtual Assembly & assembly(THREAD_ID tid) = 0;
  virtual const Assembly & assembly(THREAD_ID tid) const = 0;

  /**
   * Return the nonlinear system object as a base class reference
   */
  virtual const SystemBase & systemBaseNonlinear() const = 0;
  virtual SystemBase & systemBaseNonlinear() = 0;
  /**
   * Return the auxiliary system object as a base class reference
   */
  virtual const SystemBase & systemBaseAuxiliary() const = 0;
  virtual SystemBase & systemBaseAuxiliary() = 0;

  virtual void prepareShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) = 0;

  /**
   * Returns the desired radial direction for RZ coordinate transformation
   * @return The coordinate direction for the radial direction
   */
  unsigned int getAxisymmetricRadialCoord() const;

  virtual DiracKernelInfo & diracKernelInfo();
  virtual Real finalNonlinearResidual() const;
  virtual unsigned int nNonlinearIterations() const;
  virtual unsigned int nLinearIterations() const;

  virtual void addResidual(THREAD_ID tid) = 0;
  virtual void addResidualNeighbor(THREAD_ID tid) = 0;

  virtual void cacheResidual(THREAD_ID tid) = 0;
  virtual void cacheResidualNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedResidual(THREAD_ID tid) = 0;

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) = 0;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) = 0;

  virtual void addJacobian(THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(THREAD_ID tid) = 0;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid) = 0;

  virtual void cacheJacobian(THREAD_ID tid) = 0;
  virtual void cacheJacobianNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedJacobian(THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepareFace(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       THREAD_ID tid) = 0;
  virtual void setCurrentSubdomainID(const Elem * elem, THREAD_ID tid) = 0;
  virtual void setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void prepareAssembly(THREAD_ID tid) = 0;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              THREAD_ID tid,
                              bool suppress_displaced_init = false) = 0;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitLowerDElem(const Elem * lower_d_elem,
                                THREAD_ID tid,
                                const std::vector<Point> * const pts = nullptr,
                                const std::vector<Real> * const weights = nullptr);
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) = 0;
  /**
   * fills the VariableValue arrays for scalar variables from the solution vector
   * @param tid The thread id
   * @param reinit_for_derivative_reordering A flag indicating whether we are reinitializing for the
   *        purpose of re-ordering derivative information for ADNodalBCs
   */
  virtual void reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering = false) = 0;
  virtual void reinitOffDiagScalars(THREAD_ID tid) = 0;

  /// sets the current boundary ID in assembly
  void setCurrentBoundaryID(BoundaryID bid, THREAD_ID tid);

  /**
   * reinitialize FE objects on a given element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitElemFaceRef(const Elem * elem,
                                 unsigned int side,
                                 BoundaryID bnd_id,
                                 Real tolerance,
                                 const std::vector<Point> * const pts,
                                 const std::vector<Real> * const weights = nullptr,
                                 THREAD_ID tid = 0);

  /**
   * reinitialize FE objects on a given neighbor element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitNeighborFaceRef(const Elem * neighbor_elem,
                                     unsigned int neighbor_side,
                                     BoundaryID bnd_id,
                                     Real tolerance,
                                     const std::vector<Point> * const pts,
                                     const std::vector<Real> * const weights = nullptr,
                                     THREAD_ID tid = 0);

  /**
   * Reinit a mortar element to obtain a valid JxW
   */
  void reinitMortarElem(const Elem * elem, THREAD_ID tid = 0);

  /**
   * Returns true if the Problem has Dirac kernels it needs to compute on elem.
   */
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) = 0;
  /**
   * Fills "elems" with the elements that should be looped over for Dirac Kernels
   */
  virtual void getDiracElements(std::set<const Elem *> & elems) = 0;
  /**
   * Gets called before Dirac Kernels are asked to add the points they are supposed to be evaluated
   * in
   */
  virtual void clearDiracInfo() = 0;

  // Geom Search
  virtual void
  updateGeomSearch(GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) = 0;

  virtual GeometricSearchData & geomSearchData() = 0;

  virtual void meshChanged();

  /**
   * Adds the given material property to a storage map based on block ids
   *
   * This is method is called from within the Material class when the property
   * is first registered.
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeSubdomainMatPropName(SubdomainID block_id, const std::string & name);

  /**
   * Adds the given material property to a storage map based on boundary ids
   *
   * This is method is called from within the Material class when the property
   * is first registered.
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeBoundaryMatPropName(BoundaryID boundary_id, const std::string & name);

  /**
   * Adds to a map based on block ids of material properties for which a zero
   * value can be returned. Thes properties are optional and will not trigger a
   * missing material property error.
   *
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeSubdomainZeroMatProp(SubdomainID block_id, const MaterialPropertyName & name);

  /**
   * Adds to a map based on boundary ids of material properties for which a zero
   * value can be returned. Thes properties are optional and will not trigger a
   * missing material property error.
   *
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeBoundaryZeroMatProp(BoundaryID boundary_id, const MaterialPropertyName & name);

  /**
   * Adds to a map based on block ids of material properties to validate
   *
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeSubdomainDelayedCheckMatProp(const std::string & requestor,
                                                 SubdomainID block_id,
                                                 const std::string & name);

  /**
   * Adds to a map based on boundary ids of material properties to validate
   *
   * @param requestor The MOOSE object name requesting the material property
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeBoundaryDelayedCheckMatProp(const std::string & requestor,
                                                BoundaryID boundary_id,
                                                const std::string & name);

  /**
   * Checks block material properties integrity
   *
   * \see FEProblemBase::checkProblemIntegrity
   */
  virtual void checkBlockMatProps();

  /**
   * Checks boundary material properties integrity
   *
   * \see FEProblemBase::checkProblemIntegrity
   */
  virtual void checkBoundaryMatProps();

  /**
   * Helper method for adding a material property name to the _material_property_requested set
   */
  virtual void markMatPropRequested(const std::string &);

  /**
   * Find out if a material property has been requested by any object
   */
  virtual bool isMatPropRequested(const std::string & prop_name) const;

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(dof_id_type elem_id) = 0;

  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id) = 0;

  /**
   * Causes the boundaries added using addGhostedBoundary to actually be ghosted.
   */
  virtual void ghostGhostedBoundaries() = 0;

  /**
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::set<SubdomainID> getMaterialPropertyBlocks(const std::string & prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<SubdomainName> getMaterialPropertyBlockNames(const std::string & prop_name);

  /**
   * Check if a material property is defined on a block.
   */
  virtual bool hasBlockMaterialProperty(SubdomainID block_id, const std::string & prop_name);

  /**
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::set<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<BoundaryName> getMaterialPropertyBoundaryNames(const std::string & prop_name);

  /**
   * Check if a material property is defined on a block.
   */
  virtual bool hasBoundaryMaterialProperty(BoundaryID boundary_id, const std::string & prop_name);

  /**
   * Returns true if the problem is in the process of computing it's initial residual.
   * @return Whether or not the problem is currently computing the initial residual.
   */
  virtual bool computingInitialResidual() const = 0;

  /**
   * Return the list of elements that should have their DoFs ghosted to this processor.
   * @return The list
   */
  virtual std::set<dof_id_type> & ghostedElems() { return _ghosted_elems; }

  std::map<std::string, std::vector<dof_id_type>> _var_dof_map;
  const CouplingMatrix & nonlocalCouplingMatrix() const { return _nonlocal_cm; }

  /**
   * Returns true if the problem is in the process of computing Jacobian
   */
  virtual const bool & currentlyComputingJacobian() const { return _currently_computing_jacobian; };

  virtual void setCurrentlyComputingJacobian(const bool & flag)
  {
    _currently_computing_jacobian = flag;
  }

  /// Check whether residual being evaulated is non-linear
  bool computingNonlinearResid() const { return _computing_nonlinear_residual; }

  /// Set whether residual being evaulated is non-linear
  virtual void computingNonlinearResid(bool computing_nonlinear_residual)
  {
    _computing_nonlinear_residual = computing_nonlinear_residual;
  }

  /// Is it safe to access the tagged  matrices
  bool safeAccessTaggedMatrices() const { return _safe_access_tagged_matrices; }

  /// Is it safe to access the tagged vectors
  bool safeAccessTaggedVectors() const { return _safe_access_tagged_vectors; }

  virtual void clearActiveFEVariableCoupleableMatrixTags(THREAD_ID tid);

  virtual void clearActiveFEVariableCoupleableVectorTags(THREAD_ID tid);

  virtual void setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags, THREAD_ID tid);

  virtual void setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags, THREAD_ID tid);

  virtual void clearActiveScalarVariableCoupleableMatrixTags(THREAD_ID tid);

  virtual void clearActiveScalarVariableCoupleableVectorTags(THREAD_ID tid);

  virtual void setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags, THREAD_ID tid);

  virtual void setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags, THREAD_ID tid);

  const std::set<TagID> & getActiveScalarVariableCoupleableVectorTags(THREAD_ID tid) const;

  const std::set<TagID> & getActiveScalarVariableCoupleableMatrixTags(THREAD_ID tid) const;

  const std::set<TagID> & getActiveFEVariableCoupleableVectorTags(THREAD_ID tid) const;

  const std::set<TagID> & getActiveFEVariableCoupleableMatrixTags(THREAD_ID tid) const;

  /**
   * Method for setting whether we have any ad objects
   */
  virtual void haveADObjects(bool have_ad_objects) { _have_ad_objects = have_ad_objects; }
  /**
   * Method for reading wehther we have any ad objects
   */
  bool haveADObjects() const { return _have_ad_objects; }

  virtual LineSearch * getLineSearch() = 0;

  /**
   * The coupling matrix defining what blocks exist in the preconditioning matrix
   */
  virtual const CouplingMatrix * couplingMatrix() const = 0;

  /**
   * Add an algebraic ghosting functor to this problem's DofMaps
   */
  void addAlgebraicGhostingFunctor(GhostingFunctor & algebraic_gf, bool to_mesh = true);

  /**
   * Automatic scaling setter
   * @param automatic_scaling A boolean representing whether we are performing automatic scaling
   */
  virtual void automaticScaling(bool automatic_scaling);

  /**
   * Automatic scaling getter
   * @return A boolean representing whether we are performing automatic scaling
   */
  bool automaticScaling() const;

protected:
  /**
   * Helper function called by getVariable that handles the logic for
   * checking whether Variables of the requested type are available.
   */
  MooseVariableFieldBase & getVariableHelper(THREAD_ID tid,
                                             const std::string & var_name,
                                             Moose::VarKindType expected_var_type,
                                             Moose::VarFieldType expected_var_field_type,
                                             SystemBase & nl,
                                             SystemBase & aux);

  /**
   * Verify the integrity of _vector_tags and _typed_vector_tags
   */
  bool verifyVectorTags() const;

  /// The currently declared tags
  std::map<TagName, TagID> _matrix_tag_name_to_tag_id;

  /// Reverse map
  std::map<TagID, TagName> _matrix_tag_id_to_tag_name;

  /// The Factory for building objects
  Factory & _factory;

  CouplingMatrix _nonlocal_cm; /// nonlocal coupling matrix;

  /// Type of coordinate system per subdomain
  std::map<SubdomainID, Moose::CoordinateSystemType> _coord_sys;

  DiracKernelInfo _dirac_kernel_info;

  /// Map of material properties (block_id -> list of properties)
  std::map<SubdomainID, std::set<std::string>> _map_block_material_props;

  /// Map for boundary material properties (boundary_id -> list of properties)
  std::map<BoundaryID, std::set<std::string>> _map_boundary_material_props;

  /// Set of properties returned as zero properties
  std::map<SubdomainID, std::set<MaterialPropertyName>> _zero_block_material_props;
  std::map<BoundaryID, std::set<MaterialPropertyName>> _zero_boundary_material_props;

  /// set containing all material property names that have been requested by getMaterialProperty*
  std::set<std::string> _material_property_requested;

  ///@{
  /**
   * Data structures of the requested material properties.  We store them in a map
   * from boudnary/block id to multimap.  Each of the multimaps is a list of
   * requestor object names to material property names.
   */
  std::map<SubdomainID, std::multimap<std::string, std::string>> _map_block_material_props_check;
  std::map<BoundaryID, std::multimap<std::string, std::string>> _map_boundary_material_props_check;
  ///@}

  /// This is the set of MooseVariableFieldBase that will actually get reinited by a call to reinit(elem)
  std::vector<std::set<MooseVariableFieldBase *>> _active_elemental_moose_variables;

  /// Whether or not there is currently a list of active elemental moose variables
  /* This needs to remain <unsigned int> for threading purposes */
  std::vector<unsigned int> _has_active_elemental_moose_variables;

  /// Set of material property ids that determine whether materials get reinited
  std::vector<std::set<unsigned int>> _active_material_property_ids;

  std::vector<std::set<TagID>> _active_fe_var_coupleable_matrix_tags;

  std::vector<std::set<TagID>> _active_fe_var_coupleable_vector_tags;

  std::vector<std::set<TagID>> _active_sc_var_coupleable_matrix_tags;

  std::vector<std::set<TagID>> _active_sc_var_coupleable_vector_tags;

  /// nonlocal coupling requirement flag
  bool _requires_nonlocal_coupling;

  /// Whether or not to use default libMesh coupling
  bool _default_ghosting;

  /// Elements that should have Dofs ghosted to the local processor
  std::set<dof_id_type> _ghosted_elems;

  /// Storage for RZ axis selection
  unsigned int _rz_coord_axis;

  /// Flag to determine whether the problem is currently computing Jacobian
  bool _currently_computing_jacobian;

  /// Whether residual being evaulated is non-linear
  bool _computing_nonlinear_residual;

  /// Is it safe to retrieve data from tagged matrices
  bool _safe_access_tagged_matrices;

  /// Is it safe to retrieve data from tagged vectors
  bool _safe_access_tagged_vectors;

  /// AD flag indicating whether **any** AD objects have been added
  bool _have_ad_objects;

private:
  /// The declared vector tags
  std::vector<VectorTag> _vector_tags;

  /**
   * The vector tags assoicated with each VectorTagType
   * This is kept separate from _vector_tags for quick access into typed vector tags in places where
   * we don't want to build a new vector every call (like in residual evaluation)
   */
  std::vector<std::vector<VectorTag>> _typed_vector_tags;

  /// Map of vector tag TagName to TagID
  std::map<TagName, TagID> _vector_tags_name_map;

  bool _have_fv = false;

  ///@{ Helper functions for checking MaterialProperties
  std::string restrictionSubdomainCheckName(SubdomainID check_id);
  std::string restrictionBoundaryCheckName(BoundaryID check_id);
  ///@}

  friend class Restartable;
};

namespace Moose
{

void initial_condition(EquationSystems & es, const std::string & system_name);

} // namespace Moose
