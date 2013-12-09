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

#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include "ParallelUniqueId.h"
#include "Problem.h"
#include "DiracKernelInfo.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "RestartableData.h"

// libMesh include
#include "libmesh/equation_systems.h"
#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

class MooseMesh;
class SubProblem;
class Factory;

template<>
InputParameters validParams<SubProblem>();

/**
 * Generic class for solving transient nonlinear problems
 *
 */
class SubProblem : public Problem
{
public:
  SubProblem(const std::string & name, InputParameters parameters);
  virtual ~SubProblem();

  virtual EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache) = 0;

  virtual void solve() = 0;
  virtual bool converged() = 0;

  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual bool isTransient() const = 0;

  virtual Order getQuadratureOrder() = 0;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) = 0;
  virtual bool hasScalarVariable(const std::string & var_name) = 0;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariable *> & moose_vars, THREAD_ID tid);

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariable *> & getActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Clear the active elemental MooseVariable.  If there are no active variables then they will all be reinited.
   * Call this after finishing the computation that was using a restricted set of MooseVariables
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid);

  virtual Assembly & assembly(THREAD_ID tid) = 0;
  virtual void prepareShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) = 0;

  virtual DiracKernelInfo & diracKernelInfo();
  virtual Real finalNonlinearResidual();
  virtual unsigned int nNonlinearIterations();
  virtual unsigned int nLinearIterations();

  virtual void addResidual(THREAD_ID tid) = 0;
  virtual void addResidualNeighbor(THREAD_ID tid) = 0;

  virtual void cacheResidual(THREAD_ID tid) = 0;
  virtual void cacheResidualNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedResidual(THREAD_ID tid) = 0;

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) = 0;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) = 0;

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, std::vector<dof_id_type> & neighbor_dof_indices, THREAD_ID tid) = 0;

  virtual void cacheJacobian(THREAD_ID tid) = 0;
  virtual void cacheJacobianNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepareFace(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & dof_indices, THREAD_ID tid) = 0;
  virtual void prepareAssembly(THREAD_ID tid) = 0;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid) = 0;
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitScalars(THREAD_ID tid) = 0;

  /**
   * Returns true if the Problem has Dirac kernels it needs to compute on elem.
   */
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) = 0;
  /**
   * Fills "elems" with the elements that should be looped over for Dirac Kernels
   */
  virtual void getDiracElements(std::set<const Elem *> & elems) = 0;
  /**
   * Gets called before Dirac Kernels are asked to add the points they are supposed to be evaluated in
   */
  virtual void clearDiracInfo() = 0;

  // Geom Search
  virtual void updateGeomSearch() = 0;

  virtual GeometricSearchData & geomSearchData() = 0;

  virtual void meshChanged();

  /**
   * Adds the given material property to a storage map based on block ids
   *
   * This is method is called from within the Material class when the property
   * is begin registered.
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeMatPropName(SubdomainID block_id, const std::string & name);

  /**
   * Adds the given material property to a storage map based on boundary ids
   *
   * This is method is called from within the Material class when the property
   * is begin registered.
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeMatPropName(BoundaryID boundary_id, const std::string & name);

  /**
   * Adds to a map based on block ids of material properties to validate
   *
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeDelayedCheckMatProp(SubdomainID block_id, const std::string & name);

  /**
   * Adds to a map based on boundary ids of material properties to validate
   *
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeDelayedCheckMatProp(BoundaryID boundary_id, const std::string & name);

  /**
   * Checks block material properties integrity
   *
   * \see FEProblem::checkProblemIntegrity
   */
  virtual void checkBlockMatProps();

  /**
   * Checks boundary material properties integrity
   *
   * \see FEProblem::checkProblemIntegrity
   */
  virtual void checkBoundaryMatProps();

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(unsigned int elem_id) = 0;

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
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::set<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<BoundaryName> getMaterialPropertyBoundaryNames(const std::string & prop_name);

  /**
   * Returns true if the problem is in the process of computing it's initial residual.
   * @return Whether or not the problem is currently computing the initial residual.
   */
  virtual bool computingInitialResidual() = 0;

  /**
   * Return the list of elements that should have their DoFs ghosted to this processor.
   * @return The list
   */
  virtual std::set<unsigned int> & ghostedElems() { return _ghosted_elems; }

  /**
   * Register a piece of restartable data.  This is data that will get
   * written / read to / from a restart file.
   *
   * @param name The full (unique) name.
   * @param data The actual data object.
   * @param tid The thread id of the object.  Use 0 if the object is not threaded.
   */
  virtual void registerRestartableData(std::string name, RestartableDataValue * data, THREAD_ID tid) = 0;

public:
  /**
   * Convenience zeros
   */
  std::vector<Real> _real_zero;
  std::vector<VariableValue> _zero;
  std::vector<VariableGradient> _grad_zero;
  std::vector<VariableSecond> _second_zero;
  std::vector<VariablePhiSecond> _second_phi_zero;

protected:
  /// The Factory for building objects
  Factory & _factory;

  /// Type of coordinate system per subdomain
  std::map<SubdomainID, Moose::CoordinateSystemType> _coord_sys;

  DiracKernelInfo _dirac_kernel_info;

  /// Map of material properties (block_id -> list of properties)
  std::map<unsigned int, std::set<std::string> > _map_block_material_props;

  /// Map for boundary material properties (boundary_id -> list of properties)
  std::map<unsigned int, std::set<std::string> > _map_boundary_material_props;

  /// the map of properties requested (need to be checked)
  std::map<unsigned int, std::set<std::string> > _map_block_material_props_check;

  /// the map of properties requested (need to be checked)
  std::map<unsigned int, std::set<std::string> > _map_boundary_material_props_check;

  /// This is the set of MooseVariables that will actually get reinited by a call to reinit(elem)
  std::vector<std::set<MooseVariable *> > _active_elemental_moose_variables;

  /// Whether or not there is currently a list of active elemental moose variables
  std::vector<bool> _has_active_elemental_moose_variables;

  /// Elements that should have Dofs ghosted to the local processor
  std::set<dof_id_type> _ghosted_elems;

  /// Where the restartable data is held (indexed on tid)
  RestartableDatas _restartable_data;

private:

  /**
   * Helper method for performing material property checks
   * @param props Reference to the map of properties known
   * @param check_props Reference to the map of properties to check
   * \see checkBlockMatProps
   * \see checkBoundaryMatProps
   */
  void checkMatProps(std::map<unsigned int, std::set<std::string> > & props,
                     std::map<unsigned int, std::set<std::string> > & check_props,
                     std::string type);

  /**
   * NOTE: This is an internal function meant for MOOSE use only!
   *
   * Register a piece of recoverable data.  This is data that will get
   * written / read to / from a restart file.
   *
   * However, this data will ONLY get read from the restart file during a RECOVERY operation!
   *
   * @param name The full (unique) name.
   */
  virtual void registerRecoverableData(std::string name) = 0;

  friend class Restartable;
};

namespace Moose
{

void initial_condition(EquationSystems & es, const std::string & system_name);

} // namespace Moose

#endif /* SUBPROBLEM_H */
