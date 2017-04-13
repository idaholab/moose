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

#ifndef DISPLACEDPROBLEM_H
#define DISPLACEDPROBLEM_H

#include "SubProblem.h"
#include "DisplacedSystem.h"
#include "GeometricSearchData.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/enum_quadrature_type.h"

// Forward declarations
class MooseVariable;
class AssemblyData;
class DisplacedProblem;
class MooseMesh;
class Assembly;
class FEProblemBase;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<DisplacedProblem>();

class DisplacedProblem : public SubProblem
{
public:
  DisplacedProblem(const InputParameters & parameters);
  virtual ~DisplacedProblem();

  virtual EquationSystems & es() override { return _eq; }
  virtual MooseMesh & mesh() override { return _mesh; }
  MooseMesh & refMesh();

  DisplacedSystem & nlSys() { return _displaced_nl; }
  DisplacedSystem & auxSys() { return _displaced_aux; }

  // Return a constant reference to the vector of variable names.
  const std::vector<std::string> & getDisplacementVarNames() const { return _displacements; }

  virtual void createQRules(QuadratureType type, Order order, Order volume_order, Order face_order);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache) override;

  virtual void init() override;
  virtual void solve() override;
  virtual bool converged() override;

  /**
   * Allocate vectors and save old solutions into them.
   */
  virtual void saveOldSolutions();

  /**
   * Restore old solutions from the backup vectors and deallocate them.
   */
  virtual void restoreOldSolutions();

  /**
   * Copy the solutions on the undisplaced systems to the displaced systems.
   */
  virtual void syncSolutions();

  /**
   * Synchronize the solutions on the displaced systems to the given solutions.
   */
  virtual void syncSolutions(const NumericVector<Number> & soln,
                             const NumericVector<Number> & aux_soln);

  /**
   * Copy the solutions on the undisplaced systems to the displaced systems and
   * reinitialize the geometry search data and Dirac kernel information due to mesh displacement.
   */
  virtual void updateMesh();

  /**
   * Synchronize the solutions on the displaced systems to the given solutions and
   * reinitialize the geometry search data and Dirac kernel information due to mesh displacement.
   */
  virtual void updateMesh(const NumericVector<Number> & soln,
                          const NumericVector<Number> & aux_soln);

  virtual bool isTransient() const override;
  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) override;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) override;
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) override;
  virtual bool hasScalarVariable(const std::string & var_name) override;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual void addVariable(const std::string & var_name,
                           const FEType & type,
                           Real scale_factor,
                           const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name,
                              const FEType & type,
                              const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addScalarVariable(const std::string & var_name,
                                 Order order,
                                 Real scale_factor = 1.,
                                 const std::set<SubdomainID> * const active_subdomains = NULL);
  virtual void addAuxScalarVariable(const std::string & var_name,
                                    Order order,
                                    Real scale_factor = 1.,
                                    const std::set<SubdomainID> * const active_subdomains = NULL);

  // Adaptivity /////
  virtual void initAdaptivity();
  virtual void meshChanged() override;

  // reinit /////
  virtual void prepare(const Elem * elem, THREAD_ID tid) override;
  virtual void prepareNonlocal(THREAD_ID tid);
  virtual void prepareFace(const Elem * elem, THREAD_ID tid) override;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       THREAD_ID tid) override;
  virtual void prepareBlockNonlocal(unsigned int ivar,
                                    unsigned int jvar,
                                    const std::vector<dof_id_type> & idof_indices,
                                    const std::vector<dof_id_type> & jdof_indices,
                                    THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid) override;
  virtual void prepareAssemblyNeighbor(THREAD_ID tid);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) override;
  virtual void reinitElem(const Elem * elem, THREAD_ID tid) override;
  virtual void
  reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid) override;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNode(const Node * node, THREAD_ID tid) override;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid) override;
  virtual void reinitScalars(THREAD_ID tid) override;
  virtual void reinitOffDiagScalars(THREAD_ID tid) override;

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems) override;
  virtual void clearDiracInfo() override;

  virtual void addResidual(THREAD_ID tid) override;
  virtual void addResidualNeighbor(THREAD_ID tid) override;

  virtual void cacheResidual(THREAD_ID tid) override;
  virtual void cacheResidualNeighbor(THREAD_ID tid) override;
  virtual void addCachedResidual(THREAD_ID tid) override;

  virtual void addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) override;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) override;

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;
  virtual void addJacobianNonlocal(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) override;
  virtual void addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                        unsigned int ivar,
                                        unsigned int jvar,
                                        const DofMap & dof_map,
                                        const std::vector<dof_id_type> & idof_indices,
                                        const std::vector<dof_id_type> & jdof_indices,
                                        THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid) override;

  virtual void cacheJacobian(THREAD_ID tid) override;
  virtual void cacheJacobianNonlocal(THREAD_ID tid);
  virtual void cacheJacobianNeighbor(THREAD_ID tid) override;
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) override;

  virtual void prepareShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) override;

  virtual Assembly & assembly(THREAD_ID tid) override { return *_assembly[tid]; }

  // Geom Search /////
  virtual void updateGeomSearch(
      GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) override;
  virtual GeometricSearchData & geomSearchData() override { return _geometric_search_data; }

  virtual bool computingInitialResidual() override;

  virtual void onTimestepBegin() override;
  virtual void onTimestepEnd() override;

  /**
   * Return the list of elements that should have their DoFs ghosted to this processor.
   * @return The list
   */
  virtual std::set<dof_id_type> & ghostedElems() override;

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(dof_id_type elem_id) override;

  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   * @param boundary_id Boundary ID
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id) override;

  /**
   * Causes the boundaries added using addGhostedBoundary to actually be ghosted.
   */
  virtual void ghostGhostedBoundaries() override;

  /**
   * Resets the displaced mesh to the reference mesh.  Required when
   * refining the DisplacedMesh.
   */
  void undisplaceMesh();

protected:
  FEProblemBase & _mproblem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  /// reference mesh
  MooseMesh & _ref_mesh;
  std::vector<std::string> _displacements;

  DisplacedSystem _displaced_nl;
  DisplacedSystem _displaced_aux;

  const NumericVector<Number> * _nl_solution;
  const NumericVector<Number> * _aux_solution;

  std::vector<Assembly *> _assembly;

  GeometricSearchData _geometric_search_data;

private:
  friend class UpdateDisplacedMeshThread;
  friend class Restartable;
};

#endif /* DISPLACEDPROBLEM_H */
