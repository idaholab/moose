//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MooseError.h"
#include "FunctorMaterialProperty.h"
#include "RawValueFunctor.h"
#include "ADWrapperFunctor.h"

#include "libmesh/coupling_matrix.h"
#include "libmesh/parameters.h"

#include <memory>
#include <unordered_map>
#include <map>
#include <vector>

namespace libMesh
{
template <typename>
class VectorValue;
typedef VectorValue<Real> RealVectorValue;
class GhostingFunctor;
}

class MooseMesh;
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
class MooseObjectName;
namespace Moose
{
class FunctorEnvelopeBase;
}

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

using libMesh::CouplingMatrix;
using libMesh::EquationSystems;

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

  virtual libMesh::EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;
  virtual const MooseMesh & mesh() const = 0;
  virtual const MooseMesh & mesh(bool use_displaced) const = 0;

  /**
   * @returns whether there will be nonlocal coupling at any point in the simulation, e.g. whether
   * there are any active \emph or inactive nonlocal kernels or boundary conditions
   */
  virtual bool checkNonlocalCouplingRequirement() const = 0;

  /**
   * @return whether the given solver system \p sys_num is converged
   */
  virtual bool solverSystemConverged(const unsigned int sys_num) { return converged(sys_num); }

  /**
   * @return whether the given nonlinear system \p nl_sys_num is converged.
   */
  virtual bool nlConverged(const unsigned int nl_sys_num);

  /**
   * Eventually we want to convert this virtual over to taking a solver system number argument.
   * We will have to first convert apps to use solverSystemConverged, and then once that is done, we
   * can change this signature. Then we can go through the apps again and convert back to this
   * changed API
   */
  virtual bool converged(const unsigned int sys_num) { return solverSystemConverged(sys_num); }

  /**
   * @return the nonlinear system number corresponding to the provided \p nl_sys_name
   */
  virtual unsigned int nlSysNum(const NonlinearSystemName & nl_sys_name) const = 0;

  /**
   * @return the linear system number corresponding to the provided \p linear_sys_name
   */
  virtual unsigned int linearSysNum(const LinearSystemName & linear_sys_name) const = 0;

  /**
   * @return the solver system number corresponding to the provided \p solver_sys_name
   */
  virtual unsigned int solverSysNum(const SolverSystemName & solver_sys_name) const = 0;

  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual bool isTransient() const = 0;

  /// marks this problem as including/needing finite volume functionality.
  virtual void needFV() = 0;

  /// returns true if this problem includes/needs finite volume functionality.
  virtual bool haveFV() const = 0;

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
   * Adds a vector tag to the list of vectors that will not be zeroed
   * when other tagged vectors are
   * @param tag the TagID of the vector that will be manually managed
   */
  void addNotZeroedVectorTag(const TagID tag);

  /**
   * Checks if a vector tag is in the list of vectors that will not be zeroed
   * when other tagged vectors are
   * @param tag the TagID of the vector that is currently being checked
   * @returns false if the tag is not within the set of vectors that are
   *          intended to not be zero or if the set is empty. returns true otherwise
   */
  bool vectorTagNotZeroed(const TagID tag) const;

  /**
   * Get a VectorTag from a TagID.
   */
  virtual const VectorTag & getVectorTag(const TagID tag_id) const;
  std::vector<VectorTag> getVectorTags(const std::set<TagID> & tag_ids) const;

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
  virtual bool vectorTagExists(const TagName & tag_name) const;

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
  virtual TagID getMatrixTagID(const TagName & tag_name) const;

  /**
   * Retrieve the name associated with a TagID
   */
  virtual TagName matrixTagName(TagID tag);

  /**
   * Check to see if a particular Tag exists
   */
  virtual bool matrixTagExists(const TagName & tag_name) const;

  /**
   * Check to see if a particular Tag exists
   */
  virtual bool matrixTagExists(TagID tag_id) const;

  /**
   * The total number of tags
   */
  virtual unsigned int numMatrixTags() const { return _matrix_tag_name_to_tag_id.size(); }

  /**
   * Return all matrix tags in the system, where a tag is represented by a map from name to ID
   */
  virtual std::map<TagName, TagID> & getMatrixTags() { return _matrix_tag_name_to_tag_id; }

  /// Whether or not this problem has the variable
  virtual bool hasVariable(const std::string & var_name) const = 0;

  /// Whether or not this problem has this linear variable
  virtual bool hasLinearVariable(const std::string & var_name) const;

  /// Whether or not this problem has this auxiliary variable
  virtual bool hasAuxiliaryVariable(const std::string & var_name) const;

  /**
   * Returns the variable reference for requested variable which must
   * be of the expected_var_type (Nonlinear vs. Auxiliary) and
   * expected_var_field_type (standard, scalar, vector). The default
   * values of VAR_ANY and VAR_FIELD_ANY should be used when "any"
   * type of variable is acceptable.  Throws an error if the variable
   * in question is not in the expected System or of the expected
   * type.
   */
  virtual const MooseVariableFieldBase & getVariable(
      const THREAD_ID tid,
      const std::string & var_name,
      Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
      Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_ANY) const = 0;
  virtual MooseVariableFieldBase &
  getVariable(const THREAD_ID tid,
              const std::string & var_name,
              Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
              Moose::VarFieldType expected_var_field_type = Moose::VarFieldType::VAR_FIELD_ANY)
  {
    return const_cast<MooseVariableFieldBase &>(const_cast<const SubProblem *>(this)->getVariable(
        tid, var_name, expected_var_type, expected_var_field_type));
  }

  /// Returns the variable reference for requested MooseVariable which may be in any system
  virtual MooseVariable & getStandardVariable(const THREAD_ID tid,
                                              const std::string & var_name) = 0;

  /// Returns the variable reference for requested MooseVariableField which may be in any system
  virtual MooseVariableFieldBase & getActualFieldVariable(const THREAD_ID tid,
                                                          const std::string & var_name) = 0;

  /// Returns the variable reference for requested VectorMooseVariable which may be in any system
  virtual VectorMooseVariable & getVectorVariable(const THREAD_ID tid,
                                                  const std::string & var_name) = 0;

  /// Returns the variable reference for requested ArrayMooseVariable which may be in any system
  virtual ArrayMooseVariable & getArrayVariable(const THREAD_ID tid,
                                                const std::string & var_name) = 0;

  /// Returns a Boolean indicating whether any system contains a variable with the name provided
  virtual bool hasScalarVariable(const std::string & var_name) const = 0;

  /// Returns the scalar variable reference from whichever system contains it
  virtual MooseVariableScalar & getScalarVariable(const THREAD_ID tid,
                                                  const std::string & var_name) = 0;

  /// Returns the equation system containing the variable provided
  virtual libMesh::System & getSystem(const std::string & var_name) = 0;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void
  setActiveElementalMooseVariables(const std::set<MooseVariableFieldBase *> & moose_vars,
                                   const THREAD_ID tid);

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariableFieldBase *> &
  getActiveElementalMooseVariables(const THREAD_ID tid) const;

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(const THREAD_ID tid) const;

  /**
   * Clear the active elemental MooseVariableFieldBase.  If there are no active variables then they
   * will all be reinited. Call this after finishing the computation that was using a restricted set
   * of MooseVariableFieldBase
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(const THREAD_ID tid);

  virtual Assembly & assembly(const THREAD_ID tid, const unsigned int sys_num) = 0;
  virtual const Assembly & assembly(const THREAD_ID tid, const unsigned int sys_num) const = 0;

  /**
   * Return the nonlinear system object as a base class reference given the system number
   */
  virtual const SystemBase & systemBaseNonlinear(const unsigned int sys_num) const = 0;
  virtual SystemBase & systemBaseNonlinear(const unsigned int sys_num) = 0;
  /**
   * Return the linear system object as a base class reference given the system number
   */
  virtual const SystemBase & systemBaseLinear(const unsigned int sys_num) const = 0;
  virtual SystemBase & systemBaseLinear(const unsigned int sys_num) = 0;
  /**
   * Return the solver system object as a base class reference given the system number
   */
  virtual const SystemBase & systemBaseSolver(const unsigned int sys_num) const = 0;
  virtual SystemBase & systemBaseSolver(const unsigned int sys_num) = 0;
  /**
   * Return the auxiliary system object as a base class reference
   */
  virtual const SystemBase & systemBaseAuxiliary() const = 0;
  virtual SystemBase & systemBaseAuxiliary() = 0;

  virtual void prepareShapes(unsigned int var, const THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, const THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, const THREAD_ID tid) = 0;
  Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) const;

  /**
   * Returns the desired radial direction for RZ coordinate transformation
   * @return The coordinate direction for the radial direction
   */
  unsigned int getAxisymmetricRadialCoord() const;

  virtual DiracKernelInfo & diracKernelInfo();
  virtual Real finalNonlinearResidual(const unsigned int nl_sys_num) const;
  virtual unsigned int nNonlinearIterations(const unsigned int nl_sys_num) const;
  virtual unsigned int nLinearIterations(const unsigned int nl_sys_num) const;

  virtual void addResidual(const THREAD_ID tid) = 0;
  virtual void addResidualNeighbor(const THREAD_ID tid) = 0;
  virtual void addResidualLower(const THREAD_ID tid) = 0;

  virtual void cacheResidual(const THREAD_ID tid);
  virtual void cacheResidualNeighbor(const THREAD_ID tid);
  virtual void addCachedResidual(const THREAD_ID tid);

  virtual void setResidual(libMesh::NumericVector<libMesh::Number> & residual,
                           const THREAD_ID tid) = 0;
  virtual void setResidualNeighbor(libMesh::NumericVector<libMesh::Number> & residual,
                                   const THREAD_ID tid) = 0;

  virtual void addJacobian(const THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(const THREAD_ID tid) = 0;
  virtual void addJacobianNeighborLowerD(const THREAD_ID tid) = 0;
  virtual void addJacobianLowerD(const THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(libMesh::SparseMatrix<libMesh::Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const libMesh::DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   const std::set<TagID> & tags,
                                   const THREAD_ID tid) = 0;

  virtual void cacheJacobian(const THREAD_ID tid);
  virtual void cacheJacobianNeighbor(const THREAD_ID tid);
  virtual void addCachedJacobian(const THREAD_ID tid);

  virtual void prepare(const Elem * elem, const THREAD_ID tid) = 0;
  virtual void prepareFace(const Elem * elem, const THREAD_ID tid) = 0;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       const THREAD_ID tid) = 0;
  virtual void setCurrentSubdomainID(const Elem * elem, const THREAD_ID tid) = 0;
  virtual void
  setNeighborSubdomainID(const Elem * elem, unsigned int side, const THREAD_ID tid) = 0;
  virtual void prepareAssembly(const THREAD_ID tid) = 0;

  virtual void reinitElem(const Elem * elem, const THREAD_ID tid) = 0;
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              const THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, const THREAD_ID tid) = 0;
  virtual void reinitLowerDElem(const Elem * lower_d_elem,
                                const THREAD_ID tid,
                                const std::vector<Point> * const pts = nullptr,
                                const std::vector<Real> * const weights = nullptr);
  virtual void reinitNode(const Node * node, const THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, const THREAD_ID tid) = 0;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, const THREAD_ID tid) = 0;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, const THREAD_ID tid) = 0;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, const THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  const THREAD_ID tid) = 0;
  virtual void
  reinitElemNeighborAndLowerD(const Elem * elem, unsigned int side, const THREAD_ID tid) = 0;
  /**
   * fills the VariableValue arrays for scalar variables from the solution vector
   * @param tid The thread id
   * @param reinit_for_derivative_reordering A flag indicating whether we are reinitializing for the
   *        purpose of re-ordering derivative information for ADNodalBCs
   */
  virtual void reinitScalars(const THREAD_ID tid,
                             bool reinit_for_derivative_reordering = false) = 0;
  virtual void reinitOffDiagScalars(const THREAD_ID tid) = 0;

  /// sets the current boundary ID in assembly
  virtual void setCurrentBoundaryID(BoundaryID bid, const THREAD_ID tid);

  /**
   * reinitialize FE objects on a given element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitElemFaceRef(const Elem * elem,
                                 unsigned int side,
                                 Real tolerance,
                                 const std::vector<Point> * const pts,
                                 const std::vector<Real> * const weights = nullptr,
                                 const THREAD_ID tid = 0);

  /**
   * reinitialize FE objects on a given neighbor element on a given side at a given set of reference
   * points and then compute variable data. Note that this method makes no assumptions about what's
   * been called beforehand, e.g. you don't have to call some prepare method before this one. This
   * is an all-in-one reinit
   */
  virtual void reinitNeighborFaceRef(const Elem * neighbor_elem,
                                     unsigned int neighbor_side,
                                     Real tolerance,
                                     const std::vector<Point> * const pts,
                                     const std::vector<Real> * const weights = nullptr,
                                     const THREAD_ID tid = 0);

  /**
   * reinitialize a neighboring lower dimensional element
   */
  void reinitNeighborLowerDElem(const Elem * elem, const THREAD_ID tid = 0);

  /**
   * Reinit a mortar element to obtain a valid JxW
   */
  void reinitMortarElem(const Elem * elem, const THREAD_ID tid = 0);

  /**
   * Returns true if the Problem has Dirac kernels it needs to compute on elem.
   */
  virtual bool reinitDirac(const Elem * elem, const THREAD_ID tid) = 0;
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
   * Helper for tracking the object that is consuming a property for MaterialPropertyDebugOutput
   */
  void addConsumedPropertyName(const MooseObjectName & obj_name, const std::string & prop_name);

  /**
   * Return the map that tracks the object with consumed material properties
   */
  const std::map<MooseObjectName, std::set<std::string>> & getConsumedPropertyMap() const;

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
  virtual bool computingPreSMOResidual(const unsigned int nl_sys_num) const = 0;

  /**
   * Return the list of elements that should have their DoFs ghosted to this processor.
   * @return The list
   */
  virtual std::set<dof_id_type> & ghostedElems() { return _ghosted_elems; }

  std::map<std::string, std::vector<dof_id_type>> _var_dof_map;

  /**
   * @return the nonlocal coupling matrix for the i'th nonlinear system
   */
  virtual const libMesh::CouplingMatrix & nonlocalCouplingMatrix(const unsigned i) const = 0;

  /**
   * Returns true if the problem is in the process of computing the Jacobian
   */
  const bool & currentlyComputingJacobian() const { return _currently_computing_jacobian; };

  /**
   * Set whether or not the problem is in the process of computing the Jacobian
   */
  void setCurrentlyComputingJacobian(const bool currently_computing_jacobian)
  {
    _currently_computing_jacobian = currently_computing_jacobian;
  }

  /**
   * Returns true if the problem is in the process of computing the residual and the Jacobian
   */
  const bool & currentlyComputingResidualAndJacobian() const;

  /**
   * Set whether or not the problem is in the process of computing the Jacobian
   */
  void setCurrentlyComputingResidualAndJacobian(bool currently_computing_residual_and_jacobian);

  /**
   * Returns true if the problem is in the process of computing the nonlinear residual
   */
  bool computingNonlinearResid() const { return _computing_nonlinear_residual; }

  /**
   * Set whether or not the problem is in the process of computing the nonlinear residual
   */
  virtual void computingNonlinearResid(const bool computing_nonlinear_residual)
  {
    _computing_nonlinear_residual = computing_nonlinear_residual;
  }

  /**
   * Returns true if the problem is in the process of computing the residual
   */
  const bool & currentlyComputingResidual() const { return _currently_computing_residual; }

  /**
   * Set whether or not the problem is in the process of computing the residual
   */
  virtual void setCurrentlyComputingResidual(const bool currently_computing_residual)
  {
    _currently_computing_residual = currently_computing_residual;
  }

  /// Is it safe to access the tagged  matrices
  virtual bool safeAccessTaggedMatrices() const { return _safe_access_tagged_matrices; }

  /// Is it safe to access the tagged vectors
  virtual bool safeAccessTaggedVectors() const { return _safe_access_tagged_vectors; }

  virtual void clearActiveFEVariableCoupleableMatrixTags(const THREAD_ID tid);

  virtual void clearActiveFEVariableCoupleableVectorTags(const THREAD_ID tid);

  virtual void setActiveFEVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                       const THREAD_ID tid);

  virtual void setActiveFEVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                       const THREAD_ID tid);

  virtual void clearActiveScalarVariableCoupleableMatrixTags(const THREAD_ID tid);

  virtual void clearActiveScalarVariableCoupleableVectorTags(const THREAD_ID tid);

  virtual void setActiveScalarVariableCoupleableVectorTags(std::set<TagID> & vtags,
                                                           const THREAD_ID tid);

  virtual void setActiveScalarVariableCoupleableMatrixTags(std::set<TagID> & mtags,
                                                           const THREAD_ID tid);

  const std::set<TagID> & getActiveScalarVariableCoupleableVectorTags(const THREAD_ID tid) const;

  const std::set<TagID> & getActiveScalarVariableCoupleableMatrixTags(const THREAD_ID tid) const;

  const std::set<TagID> & getActiveFEVariableCoupleableVectorTags(const THREAD_ID tid) const;

  const std::set<TagID> & getActiveFEVariableCoupleableMatrixTags(const THREAD_ID tid) const;

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
  virtual const libMesh::CouplingMatrix * couplingMatrix(const unsigned int nl_sys_num) const = 0;

private:
  /**
   * Creates (n_sys - 1) clones of the provided algebraic ghosting functor (corresponding to the
   * nonlinear system algebraic ghosting functor), initializes the clone with the appropriate
   * DofMap, and then adds the clone to said DofMap
   * @param algebraic_gf the (nonlinear system's) algebraic ghosting functor to clone
   * @param to_mesh whether the clone should be added to the corresponding DofMap's underlying
   * MeshBase (the underlying MeshBase will be the same for every system held by this object's
   * EquationSystems object)
   */
  void cloneAlgebraicGhostingFunctor(libMesh::GhostingFunctor & algebraic_gf, bool to_mesh = true);

  /**
   * Creates (n_sys - 1) clones of the provided coupling ghosting functor (corresponding to the
   * nonlinear system coupling ghosting functor), initializes the clone with the appropriate
   * DofMap, and then adds the clone to said DofMap
   * @param coupling_gf the (nonlinear system's) coupling ghosting functor to clone
   * @param to_mesh whether the clone should be added to the corresponding DofMap's underlying
   * MeshBase (the underlying MeshBase will be the same for every system held by this object's
   * EquationSystems object)
   */
  void cloneCouplingGhostingFunctor(libMesh::GhostingFunctor & coupling_gf, bool to_mesh = true);

public:
  /**
   * Add an algebraic ghosting functor to this problem's DofMaps
   */
  void addAlgebraicGhostingFunctor(libMesh::GhostingFunctor & algebraic_gf, bool to_mesh = true);

  /**
   * Add a coupling functor to this problem's DofMaps
   */
  void addCouplingGhostingFunctor(libMesh::GhostingFunctor & coupling_gf, bool to_mesh = true);

  /**
   * Remove an algebraic ghosting functor from this problem's DofMaps
   */
  void removeAlgebraicGhostingFunctor(libMesh::GhostingFunctor & algebraic_gf);

  /**
   * Remove a coupling ghosting functor from this problem's DofMaps
   */
  void removeCouplingGhostingFunctor(libMesh::GhostingFunctor & coupling_gf);

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

  /**
   * Tells this problem that the assembly associated with the given nonlinear system number involves
   * a scaling vector
   */
  void hasScalingVector(const unsigned int nl_sys_num);

  /**
   * Whether we have a displaced problem in our simulation
   */
  virtual bool haveDisplaced() const = 0;

  /**
   * Getter for whether we're computing the scaling jacobian
   */
  virtual bool computingScalingJacobian() const = 0;

  /**
   * Getter for whether we're computing the scaling residual
   */
  virtual bool computingScalingResidual() const = 0;

  /**
   * Clear dof indices from variables in nl and aux systems
   */
  void clearAllDofIndices();

  /**
   * @tparam T The type that the functor will return when evaluated, e.g. \p
               ADReal or \p Real
   * @param name The name of the functor to retrieve
   * @param tid The thread ID that we are retrieving the functor property for
   * @param requestor_name The name of the object that is requesting this functor property
   * @param requestor_is_ad Whether the requesting object is an AD object
   * @return a constant reference to the functor
   */
  template <typename T>
  const Moose::Functor<T> & getFunctor(const std::string & name,
                                       const THREAD_ID tid,
                                       const std::string & requestor_name,
                                       bool requestor_is_ad);

  /**
   * checks whether we have a functor corresponding to \p name on the thread id \p tid
   */
  bool hasFunctor(const std::string & name, const THREAD_ID tid) const;

  /**
   * checks whether we have a functor of type T corresponding to \p name on the thread id \p tid
   */
  template <typename T>
  bool hasFunctorWithType(const std::string & name, const THREAD_ID tid) const;

  /**
   * add a functor to the problem functor container
   */
  template <typename T>
  void
  addFunctor(const std::string & name, const Moose::FunctorBase<T> & functor, const THREAD_ID tid);

  /**
   * Add a functor that has block-wise lambda definitions, e.g. the evaluations of the functor are
   * based on a user-provided lambda expression.
   * @param name The name of the functor to add
   * @param my_lammy The lambda expression that will be called when the functor is evaluated
   * @param clearance_schedule How often to clear functor evaluations. The default value is always,
   * which means that the functor will be re-evaluated every time it is called. If it is something
   * other than always, than cached values may be returned
   * @param mesh The mesh on which this functor operates
   * @param block_ids The blocks on which the lambda expression is defined
   * @param tid The thread on which the functor we are adding will run
   * @return The added functor
   */
  template <typename T, typename PolymorphicLambda>
  const Moose::FunctorBase<T> &
  addPiecewiseByBlockLambdaFunctor(const std::string & name,
                                   PolymorphicLambda my_lammy,
                                   const std::set<ExecFlagType> & clearance_schedule,
                                   const MooseMesh & mesh,
                                   const std::set<SubdomainID> & block_ids,
                                   const THREAD_ID tid);

  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void customSetup(const ExecFlagType & exec_type);
  virtual void residualSetup();
  virtual void jacobianSetup();

  /// Setter for debug functor output
  void setFunctorOutput(bool set_output) { _output_functors = set_output; }

  /**
   * @return the number of nonlinear systems in the problem
   */
  virtual std::size_t numNonlinearSystems() const = 0;

  /**
   * @return the current nonlinear system number
   */
  virtual unsigned int currentNlSysNum() const = 0;

  /**
   * @return the number of linear systems in the problem
   */
  virtual std::size_t numLinearSystems() const = 0;

  /**
   * @return the number of solver systems in the problem
   */
  virtual std::size_t numSolverSystems() const = 0;

  /**
   * @return the current linear system number
   */
  virtual unsigned int currentLinearSysNum() const = 0;

  /**
   * Register an unfulfilled functor request
   */
  template <typename T>
  void registerUnfilledFunctorRequest(T * functor_interface,
                                      const std::string & functor_name,
                                      const THREAD_ID tid);

  /**
   * Return the residual vector tags we are currently computing
   */
  virtual const std::vector<VectorTag> & currentResidualVectorTags() const = 0;

  /**
   * Select the vector tags which belong to a specific system
   * @param system Reference to the system
   * @param input_vector_tags A vector of vector tags
   * @param selected_tags A set which gets populated by the tag-ids that belong to the system
   */
  static void selectVectorTagsFromSystem(const SystemBase & system,
                                         const std::vector<VectorTag> & input_vector_tags,
                                         std::set<TagID> & selected_tags);

  /**
   * Select the matrix tags which belong to a specific system
   * @param system Reference to the system
   * @param input_matrix_tags A map of matrix tags
   * @param selected_tags A set which gets populated by the tag-ids that belong to the system
   */
  static void selectMatrixTagsFromSystem(const SystemBase & system,
                                         const std::map<TagName, TagID> & input_matrix_tags,
                                         std::set<TagID> & selected_tags);

  /**
   * reinitialize the finite volume assembly data for the provided face and thread
   */
  void reinitFVFace(const THREAD_ID tid, const FaceInfo & fi);

  /**
   * Whether the simulation has active nonlocal coupling which should be accounted for in the
   * Jacobian. For this to return true, there must be at least one active nonlocal kernel or
   * boundary condition
   */
  virtual bool hasNonlocalCoupling() const = 0;

  /**
   * Prepare \p DofMap and \p Assembly classes with our p-refinement information
   */
  void preparePRefinement();

  /**
   * @returns whether we're doing p-refinement
   */
  [[nodiscard]] bool doingPRefinement() const;

  /**
   * Query whether p-refinement has been requested at any point during the simulation
   */
  [[nodiscard]] bool havePRefinement() const { return _have_p_refinement; }

  /**
   * Set the current lower dimensional element. This can be null
   */
  virtual void setCurrentLowerDElem(const Elem * const lower_d_elem, const THREAD_ID tid);

protected:
  /**
   * Helper function called by getVariable that handles the logic for
   * checking whether Variables of the requested type are available.
   */
  template <typename T>
  MooseVariableFieldBase & getVariableHelper(const THREAD_ID tid,
                                             const std::string & var_name,
                                             Moose::VarKindType expected_var_type,
                                             Moose::VarFieldType expected_var_field_type,
                                             const std::vector<T> & nls,
                                             const SystemBase & aux) const;

  /**
   * Verify the integrity of _vector_tags and _typed_vector_tags
   */
  bool verifyVectorTags() const;

  /**
   * Mark a variable family for either disabling or enabling p-refinement with valid parameters of a
   * variable
   */
  void markFamilyPRefinement(const InputParameters & params);

  /// The currently declared tags
  std::map<TagName, TagID> _matrix_tag_name_to_tag_id;

  /// Reverse map
  std::map<TagID, TagName> _matrix_tag_id_to_tag_name;

  /// The Factory for building objects
  Factory & _factory;

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
   * from boundary/block id to multimap.  Each of the multimaps is a list of
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

  std::vector<std::set<TagID>> _active_fe_var_coupleable_matrix_tags;

  std::vector<std::set<TagID>> _active_fe_var_coupleable_vector_tags;

  std::vector<std::set<TagID>> _active_sc_var_coupleable_matrix_tags;

  std::vector<std::set<TagID>> _active_sc_var_coupleable_vector_tags;

  /// Whether or not to use default libMesh coupling
  bool _default_ghosting;

  /// Elements that should have Dofs ghosted to the local processor
  std::set<dof_id_type> _ghosted_elems;

  /// Flag to determine whether the problem is currently computing Jacobian
  bool _currently_computing_jacobian;

  /// Flag to determine whether the problem is currently computing the residual and Jacobian
  bool _currently_computing_residual_and_jacobian;

  /// Whether the non-linear residual is being evaluated
  bool _computing_nonlinear_residual;

  /// Whether the residual is being evaluated
  bool _currently_computing_residual;

  /// Is it safe to retrieve data from tagged matrices
  bool _safe_access_tagged_matrices;

  /// Is it safe to retrieve data from tagged vectors
  bool _safe_access_tagged_vectors;

  /// AD flag indicating whether **any** AD objects have been added
  bool _have_ad_objects;

  /// the list of vector tags that will not be zeroed when all other tags are
  std::unordered_set<TagID> _not_zeroed_tagged_vectors;

private:
  /**
   * @return whether a given variable name is in the solver systems (reflected by the first
   * member of the returned pair which is a boolean) and if so, what solver system number it is
   * in (the second member of the returned pair; if the variable is not in the solver systems,
   * then this will be an invalid unsigned integer)
   */
  virtual std::pair<bool, unsigned int>
  determineSolverSystem(const std::string & var_name, bool error_if_not_found = false) const = 0;

  enum class TrueFunctorIs
  {
    UNSET,
    NONAD,
    AD
  };

  /// A container holding pointers to all the functors in our problem. We hold a tuple where the
  /// zeroth item in the tuple is an enumerator that describes what type of functor the "true"
  /// functor is (either NONAD or AD), the first item in the tuple is the non-AD version of the
  /// functor, and the second item in the tuple is the AD version of the functor
  std::vector<std::multimap<std::string,
                            std::tuple<TrueFunctorIs,
                                       std::unique_ptr<Moose::FunctorEnvelopeBase>,
                                       std::unique_ptr<Moose::FunctorEnvelopeBase>>>>
      _functors;

  /// Container to hold PiecewiseByBlockLambdaFunctors
  std::vector<std::map<std::string, std::unique_ptr<Moose::FunctorAbstract>>> _pbblf_functors;

  /// Lists all functors in the problem
  void showFunctors() const;

  /// Lists all functors and all the objects that requested them
  void showFunctorRequestors() const;

  /// The requestors of functors where the key is the prop name and the value is a set of names of
  /// requestors
  std::map<std::string, std::set<std::string>> _functor_to_requestors;

  /// A multimap (for each thread) from unfilled functor requests to whether the requests were for
  ///  AD functors and whether the requestor was an AD object
  std::vector<std::multimap<std::string, std::pair<bool, bool>>> _functor_to_request_info;

  /// Whether to output a list of the functors used and requested (currently only at initialSetup)
  bool _output_functors;

  /// The declared vector tags
  std::vector<VectorTag> _vector_tags;

  /**
   * The vector tags associated with each VectorTagType
   * This is kept separate from _vector_tags for quick access into typed vector tags in places where
   * we don't want to build a new vector every call (like in residual evaluation)
   */
  std::vector<std::vector<VectorTag>> _typed_vector_tags;

  /// Map of vector tag TagName to TagID
  std::map<TagName, TagID> _vector_tags_name_map;

  ///@{ Helper functions for checking MaterialProperties
  std::string restrictionSubdomainCheckName(SubdomainID check_id);
  std::string restrictionBoundaryCheckName(BoundaryID check_id);
  ///@}

  // Contains properties consumed by objects, see addConsumedPropertyName
  std::map<MooseObjectName, std::set<std::string>> _consumed_material_properties;

  /// A map from a root algebraic ghosting functor, e.g. the ghosting functor passed into \p
  /// removeAlgebraicGhostingFunctor, to its clones in other systems, e.g. systems other than system
  /// 0
  std::unordered_map<libMesh::GhostingFunctor *,
                     std::vector<std::shared_ptr<libMesh::GhostingFunctor>>>
      _root_alg_gf_to_sys_clones;

  /// A map from a root coupling ghosting functor, e.g. the ghosting functor passed into \p
  /// removeCouplingGhostingFunctor, to its clones in other systems, e.g. systems other than system
  /// 0
  std::unordered_map<libMesh::GhostingFunctor *,
                     std::vector<std::shared_ptr<libMesh::GhostingFunctor>>>
      _root_coupling_gf_to_sys_clones;

  /// Whether p-refinement has been requested at any point during the simulation
  bool _have_p_refinement;

  /// Indicate whether a family is disabled for p-refinement
  std::unordered_map<FEFamily, bool> _family_for_p_refinement;
  /// The set of variable families by default disable p-refinement
  static const std::unordered_set<FEFamily> _default_families_without_p_refinement;

  friend class Restartable;
};

template <typename T>
const Moose::Functor<T> &
SubProblem::getFunctor(const std::string & name,
                       const THREAD_ID tid,
                       const std::string & requestor_name,
                       const bool requestor_is_ad)
{
  mooseAssert(tid < _functors.size(), "Too large a thread ID");

  // Log the requestor
  _functor_to_requestors["wraps_" + name].insert(requestor_name);

  constexpr bool requested_functor_is_ad =
      !std::is_same<T, typename MetaPhysicL::RawType<T>::value_type>::value;

  auto & functor_to_request_info = _functor_to_request_info[tid];

  // Get the requested functor if we already have it
  auto & functors = _functors[tid];
  if (auto find_ret = functors.find("wraps_" + name); find_ret != functors.end())
  {
    if (functors.count("wraps_" + name) > 1)
      mooseError("Attempted to get a functor with the name '",
                 name,
                 "' but multiple (" + std::to_string(functors.count("wraps_" + name)) +
                     ") functors match. Make sure that you do not have functor material "
                     "properties, functions, postprocessors or variables with the same names.");

    auto & [true_functor_is, non_ad_functor, ad_functor] = find_ret->second;
    auto & functor_wrapper = requested_functor_is_ad ? *ad_functor : *non_ad_functor;

    auto * const functor = dynamic_cast<Moose::Functor<T> *>(&functor_wrapper);
    if (!functor)
      mooseError("A call to SubProblem::getFunctor requested a functor named '",
                 name,
                 "' that returns the type: '",
                 libMesh::demangle(typeid(T).name()),
                 "'. However, that functor already exists and returns a different type: '",
                 functor_wrapper.returnType(),
                 "'");

    if (functor->template wrapsType<Moose::NullFunctor<T>>())
      // Store for future checking when the actual functor gets added
      functor_to_request_info.emplace(name,
                                      std::make_pair(requested_functor_is_ad, requestor_is_ad));
    else
    {
      // We already have the actual functor
      if (true_functor_is == SubProblem::TrueFunctorIs::UNSET)
        mooseError("We already have the functor; it should not be unset");

      // Check for whether this is a valid request
      // We allow auxiliary variables and linear variables to be retrieved as non AD
      if (!requested_functor_is_ad && requestor_is_ad &&
          true_functor_is == SubProblem::TrueFunctorIs::AD &&
          !(hasAuxiliaryVariable(name) || hasLinearVariable(name)))
        mooseError("The AD object '",
                   requestor_name,
                   "' is requesting the functor '",
                   name,
                   "' as a non-AD functor even though it is truly an AD functor, which is not "
                   "allowed, since this may unintentionally drop derivatives.");
    }

    return *functor;
  }

  // We don't have the functor yet but we could have it in the future. We'll create null functors
  // for now
  functor_to_request_info.emplace(name, std::make_pair(requested_functor_is_ad, requestor_is_ad));
  if constexpr (requested_functor_is_ad)
  {
    typedef typename MetaPhysicL::RawType<T>::value_type NonADType;
    typedef T ADType;

    auto emplace_ret =
        functors.emplace("wraps_" + name,
                         std::make_tuple(SubProblem::TrueFunctorIs::UNSET,
                                         std::make_unique<Moose::Functor<NonADType>>(
                                             std::make_unique<Moose::NullFunctor<NonADType>>()),
                                         std::make_unique<Moose::Functor<ADType>>(
                                             std::make_unique<Moose::NullFunctor<ADType>>())));

    return static_cast<Moose::Functor<T> &>(*(requested_functor_is_ad
                                                  ? std::get<2>(emplace_ret->second)
                                                  : std::get<1>(emplace_ret->second)));
  }
  else
  {
    typedef T NonADType;
    typedef typename Moose::ADType<T>::type ADType;

    auto emplace_ret =
        functors.emplace("wraps_" + name,
                         std::make_tuple(SubProblem::TrueFunctorIs::UNSET,
                                         std::make_unique<Moose::Functor<NonADType>>(
                                             std::make_unique<Moose::NullFunctor<NonADType>>()),
                                         std::make_unique<Moose::Functor<ADType>>(
                                             std::make_unique<Moose::NullFunctor<ADType>>())));

    return static_cast<Moose::Functor<T> &>(*(requested_functor_is_ad
                                                  ? std::get<2>(emplace_ret->second)
                                                  : std::get<1>(emplace_ret->second)));
  }
}

template <typename T>
bool
SubProblem::hasFunctorWithType(const std::string & name, const THREAD_ID tid) const
{
  mooseAssert(tid < _functors.size(), "Too large a thread ID");
  auto & functors = _functors[tid];

  const auto & it = functors.find("wraps_" + name);
  constexpr bool requested_functor_is_ad =
      !std::is_same<T, typename MetaPhysicL::RawType<T>::value_type>::value;

  if (it == functors.end())
    return false;
  else
    return dynamic_cast<Moose::Functor<T> *>(
        requested_functor_is_ad ? std::get<2>(it->second).get() : std::get<1>(it->second).get());
}

template <typename T, typename PolymorphicLambda>
const Moose::FunctorBase<T> &
SubProblem::addPiecewiseByBlockLambdaFunctor(const std::string & name,
                                             PolymorphicLambda my_lammy,
                                             const std::set<ExecFlagType> & clearance_schedule,
                                             const MooseMesh & mesh,
                                             const std::set<SubdomainID> & block_ids,
                                             const THREAD_ID tid)
{
  auto & pbblf_functors = _pbblf_functors[tid];

  auto [it, first_time_added] =
      pbblf_functors.emplace(name,
                             std::make_unique<PiecewiseByBlockLambdaFunctor<T>>(
                                 name, my_lammy, clearance_schedule, mesh, block_ids));

  auto * functor = dynamic_cast<PiecewiseByBlockLambdaFunctor<T> *>(it->second.get());
  if (!functor)
  {
    if (first_time_added)
      mooseError("This should be impossible. If this was the first time we added the functor, then "
                 "the dynamic cast absolutely should have succeeded");
    else
      mooseError("Attempted to add a lambda functor with the name '",
                 name,
                 "' but another lambda functor of that name returns a different type");
  }

  if (first_time_added)
    addFunctor(name, *functor, tid);
  else
    // The functor already exists
    functor->setFunctor(mesh, block_ids, my_lammy);

  return *functor;
}

template <typename T>
void
SubProblem::addFunctor(const std::string & name,
                       const Moose::FunctorBase<T> & functor,
                       const THREAD_ID tid)
{
  constexpr bool added_functor_is_ad =
      !std::is_same<T, typename MetaPhysicL::RawType<T>::value_type>::value;

  mooseAssert(tid < _functors.size(), "Too large a thread ID");

  auto & functor_to_request_info = _functor_to_request_info[tid];
  auto & functors = _functors[tid];
  auto it = functors.find("wraps_" + name);
  if (it != functors.end())
  {
    // We have this functor already. If it's a null functor, we want to replace it with the valid
    // functor we have now. If it's not then we'll add a new entry into the multimap and then we'll
    // error later if a user requests a functor because their request is ambiguous. This is the
    // reason that the functors container is a multimap: for nice error messages
    auto * const existing_wrapper_base =
        added_functor_is_ad ? std::get<2>(it->second).get() : std::get<1>(it->second).get();
    auto * const existing_wrapper = dynamic_cast<Moose::Functor<T> *>(existing_wrapper_base);
    if (existing_wrapper && existing_wrapper->template wrapsType<Moose::NullFunctor<T>>())
    {
      // Sanity check
      auto [request_info_it, request_info_end_it] = functor_to_request_info.equal_range(name);
      if (request_info_it == request_info_end_it)
        mooseError("We are wrapping a NullFunctor but we don't have any unfilled functor request "
                   "info. This doesn't make sense.");

      // Check for valid requests
      while (request_info_it != request_info_end_it)
      {
        auto & [requested_functor_is_ad, requestor_is_ad] = request_info_it->second;
        if (!requested_functor_is_ad && requestor_is_ad && added_functor_is_ad)
          mooseError("We are requesting a non-AD functor '" + name +
                     "' from an AD object, but the true functor is AD. This means we could be "
                     "dropping important derivatives. We will not allow this");
        // We're going to eventually check whether we've fulfilled all functor requests and our
        // check will be that the multimap is empty. This request is fulfilled, so erase it from the
        // map now
        request_info_it = functor_to_request_info.erase(request_info_it);
      }

      // Ok we didn't have the functor before, so we will add it now
      std::get<0>(it->second) =
          added_functor_is_ad ? SubProblem::TrueFunctorIs::AD : SubProblem::TrueFunctorIs::NONAD;
      existing_wrapper->assign(functor);
      // Finally we create the non-AD or AD complement of the just added functor
      if constexpr (added_functor_is_ad)
      {
        typedef typename MetaPhysicL::RawType<T>::value_type NonADType;
        auto * const existing_non_ad_wrapper_base = std::get<1>(it->second).get();
        auto * const existing_non_ad_wrapper =
            dynamic_cast<Moose::Functor<NonADType> *>(existing_non_ad_wrapper_base);
        mooseAssert(existing_non_ad_wrapper->template wrapsType<Moose::NullFunctor<NonADType>>(),
                    "Both members of pair should have been wrapping a NullFunctor");
        existing_non_ad_wrapper->assign(
            std::make_unique<Moose::RawValueFunctor<NonADType>>(functor));
      }
      else
      {
        typedef typename Moose::ADType<T>::type ADType;
        auto * const existing_ad_wrapper_base = std::get<2>(it->second).get();
        auto * const existing_ad_wrapper =
            dynamic_cast<Moose::Functor<ADType> *>(existing_ad_wrapper_base);
        mooseAssert(existing_ad_wrapper->template wrapsType<Moose::NullFunctor<ADType>>(),
                    "Both members of pair should have been wrapping a NullFunctor");
        existing_ad_wrapper->assign(std::make_unique<Moose::ADWrapperFunctor<ADType>>(functor));
      }
      return;
    }
    else if (!existing_wrapper)
    {
      // Functor was emplaced but the cast failed. This could be a double definition with
      // different types, or it could be a request with one type then a definition with another
      // type. Either way it is going to error later, but it is cleaner to catch it now
      mooseError("Functor '",
                 name,
                 "' is being added with return type '",
                 MooseUtils::prettyCppType<T>(),
                 "' but it has already been defined or requested with return type '",
                 existing_wrapper_base->returnType(),
                 "'.");
    }
  }

  // We are a new functor, create the opposite ADType one and store it with other functors
  if constexpr (added_functor_is_ad)
  {
    typedef typename MetaPhysicL::RawType<T>::value_type NonADType;
    auto new_non_ad_wrapper = std::make_unique<Moose::Functor<NonADType>>(
        std::make_unique<Moose::RawValueFunctor<NonADType>>(functor));
    auto new_ad_wrapper = std::make_unique<Moose::Functor<T>>(functor);
    _functors[tid].emplace("wraps_" + name,
                           std::make_tuple(SubProblem::TrueFunctorIs::AD,
                                           std::move(new_non_ad_wrapper),
                                           std::move(new_ad_wrapper)));
  }
  else
  {
    typedef typename Moose::ADType<T>::type ADType;
    auto new_non_ad_wrapper = std::make_unique<Moose::Functor<T>>((functor));
    auto new_ad_wrapper = std::make_unique<Moose::Functor<ADType>>(
        std::make_unique<Moose::ADWrapperFunctor<ADType>>(functor));
    _functors[tid].emplace("wraps_" + name,
                           std::make_tuple(SubProblem::TrueFunctorIs::NONAD,
                                           std::move(new_non_ad_wrapper),
                                           std::move(new_ad_wrapper)));
  }
}

inline const bool &
SubProblem::currentlyComputingResidualAndJacobian() const
{
  return _currently_computing_residual_and_jacobian;
}

inline void
SubProblem::setCurrentlyComputingResidualAndJacobian(
    const bool currently_computing_residual_and_jacobian)
{
  _currently_computing_residual_and_jacobian = currently_computing_residual_and_jacobian;
}

namespace Moose
{
void initial_condition(libMesh::EquationSystems & es, const std::string & system_name);
} // namespace Moose
