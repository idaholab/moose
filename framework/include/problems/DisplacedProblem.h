//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubProblem.h"
#include "DisplacedSystem.h"
#include "GeometricSearchData.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/enum_quadrature_type.h"

// Forward declarations
class MooseVariableFieldBase;
class AssemblyData;
class MooseMesh;
class Assembly;
class FEProblemBase;
class LineSearch;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class DisplacedProblem : public SubProblem
{
public:
  static InputParameters validParams();

  DisplacedProblem(DisplacedProblem &&) = delete;
  DisplacedProblem & operator=(DisplacedProblem &&) = delete;

  DisplacedProblem(const InputParameters & parameters);
  ~DisplacedProblem();

  virtual EquationSystems & es() override { return _eq; }
  virtual MooseMesh & mesh() override { return _mesh; }
  virtual const MooseMesh & mesh() const override { return _mesh; }
  const MooseMesh & mesh(bool libmesh_dbg_var(use_displaced)) const override
  {
    mooseAssert(use_displaced, "An undisplaced mesh was queried from the displaced problem");
    return mesh();
  }
  MooseMesh & refMesh();

  DisplacedSystem & nlSys(unsigned int sys_num = 0);
  DisplacedSystem & auxSys() { return *_displaced_aux; }

  virtual const SystemBase & systemBaseNonlinear(unsigned int sys_num = 0) const override;
  virtual SystemBase & systemBaseNonlinear(unsigned int sys_num = 0) override;

  virtual const SystemBase & systemBaseAuxiliary() const override { return *_displaced_aux; }
  virtual SystemBase & systemBaseAuxiliary() override { return *_displaced_aux; }

  // Return a constant reference to the vector of variable names.
  const std::vector<std::string> & getDisplacementVarNames() const { return _displacements; }

  virtual void createQRules(QuadratureType type,
                            Order order,
                            Order volume_order,
                            Order face_order,
                            SubdomainID block,
                            bool allow_negative_qweights = true);

  void bumpVolumeQRuleOrder(Order order, SubdomainID block);
  void bumpAllQRuleOrder(Order order, SubdomainID block);

  virtual void init() override;
  virtual bool nlConverged(unsigned int nl_sys_num) override;

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
   * Synchronize the solutions on the displaced systems to the given solutions. The nonlinear
   * solutions argument is a map from the nonlinear system number to the solution that we want to
   * set that nonlinear system's solution to
   */
  virtual void syncSolutions(const std::map<unsigned int, const NumericVector<Number> *> & nl_solns,
                             const NumericVector<Number> & aux_soln);

  /**
   * Copy the solutions on the undisplaced systems to the displaced systems and
   * reinitialize the geometry search data and Dirac kernel information due to mesh displacement.
   * The parameter \p mesh_changing indicates whether this method is getting called because of mesh
   * changes, e.g. due to mesh adaptivity. If \p mesh_changing we need to renitialize the
   * GeometricSearchData instead of simply update. Reinitialization operations are a super-set of
   * update operations. Reinitialization for example re-generates neighbor nodes in
   * NearestNodeLocators, while update does not. Additionally we do not want to use the undisplaced
   * mesh solution because it may be out-of-sync, whereas our displaced mesh solution should be in
   * the correct state after getting restricted/prolonged in EquationSystems::reinit
   */
  virtual void updateMesh(bool mesh_changing = false);

  /**
   * Synchronize the solutions on the displaced systems to the given solutions and
   * reinitialize the geometry search data and Dirac kernel information due to mesh displacement.
   */
  virtual void updateMesh(const std::map<unsigned int, const NumericVector<Number> *> & nl_soln,
                          const NumericVector<Number> & aux_soln);

  virtual TagID addVectorTag(const TagName & tag_name,
                             const Moose::VectorTagType type = Moose::VECTOR_TAG_RESIDUAL) override;
  virtual const VectorTag & getVectorTag(const TagID tag_id) const override;
  virtual TagID getVectorTagID(const TagName & tag_name) const override;
  virtual TagName vectorTagName(const TagID tag_id) const override;
  virtual bool vectorTagExists(const TagID tag_id) const override;
  virtual bool vectorTagExists(const TagName & tag_name) const override;
  virtual unsigned int
  numVectorTags(const Moose::VectorTagType type = Moose::VECTOR_TAG_ANY) const override;
  virtual const std::vector<VectorTag> &
  getVectorTags(const Moose::VectorTagType type = Moose::VECTOR_TAG_ANY) const override;
  virtual Moose::VectorTagType vectorTagType(const TagID tag_id) const override;

  virtual TagID addMatrixTag(TagName tag_name) override;
  virtual TagID getMatrixTagID(const TagName & tag_name) override;
  virtual TagName matrixTagName(TagID tag) override;
  virtual bool matrixTagExists(const TagName & tag_name) override;
  virtual bool matrixTagExists(TagID tag_id) override;
  virtual unsigned int numMatrixTags() const override;

  virtual bool isTransient() const override;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) const override;
  using SubProblem::getVariable;
  virtual const MooseVariableFieldBase &
  getVariable(THREAD_ID tid,
              const std::string & var_name,
              Moose::VarKindType expected_var_type = Moose::VarKindType::VAR_ANY,
              Moose::VarFieldType expected_var_field_type =
                  Moose::VarFieldType::VAR_FIELD_ANY) const override;
  virtual MooseVariable & getStandardVariable(THREAD_ID tid, const std::string & var_name) override;
  virtual MooseVariableFieldBase & getActualFieldVariable(THREAD_ID tid,
                                                          const std::string & var_name) override;
  virtual VectorMooseVariable & getVectorVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual ArrayMooseVariable & getArrayVariable(THREAD_ID tid,
                                                const std::string & var_name) override;
  virtual bool hasScalarVariable(const std::string & var_name) const override;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid,
                                                  const std::string & var_name) override;
  virtual System & getSystem(const std::string & var_name) override;

  virtual void addVariable(const std::string & var_type,
                           const std::string & name,
                           InputParameters & parameters,
                           unsigned int nl_system_number);
  virtual void addAuxVariable(const std::string & var_type,
                              const std::string & name,
                              InputParameters & parameters);
  //
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
  virtual void setCurrentSubdomainID(const Elem * elem, THREAD_ID tid) override;
  virtual void setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid) override;
  virtual void prepareBlockNonlocal(unsigned int ivar,
                                    unsigned int jvar,
                                    const std::vector<dof_id_type> & idof_indices,
                                    const std::vector<dof_id_type> & jdof_indices,
                                    THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid) override;
  virtual void prepareAssemblyNeighbor(THREAD_ID tid);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) override;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) override;
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              THREAD_ID tid) override;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNode(const Node * node, THREAD_ID tid) override;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) override;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid) override;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) override;

  /**
   * reinitialize neighbor routine
   * @param elem The element driving the reinit (note that this is not the *neighbor*)
   * @param side The side, e.g. face,  of the \p elem that we want to reinit
   * @param tid The thread for which we are reiniting
   * @param neighbor_reference_points Specify the referrence points for the
   *                                  neighbor element. Useful if the element and neighbor faces are
   *                                  not coincident
   */
  void reinitNeighbor(const Elem * elem,
                      unsigned int side,
                      THREAD_ID tid,
                      const std::vector<Point> * neighbor_reference_points);

  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) override;
  virtual void
  reinitElemNeighborAndLowerD(const Elem * elem, unsigned int side, THREAD_ID tid) override;
  virtual void reinitScalars(THREAD_ID tid, bool reinit_for_derivative_reordering = false) override;
  virtual void reinitOffDiagScalars(THREAD_ID tid) override;

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems) override;
  virtual void clearDiracInfo() override;

  virtual void addResidual(THREAD_ID tid) override;
  virtual void addResidualNeighbor(THREAD_ID tid) override;
  virtual void addResidualLower(THREAD_ID tid) override;

  virtual void cacheResidual(THREAD_ID tid) override;
  virtual void cacheResidualNeighbor(THREAD_ID tid) override;
  virtual void addCachedResidual(THREAD_ID tid) override;

  virtual void addCachedResidualDirectly(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) override;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) override;

  virtual void addJacobian(THREAD_ID tid) override;
  virtual void addJacobianNonlocal(THREAD_ID tid);
  virtual void addJacobianNeighbor(THREAD_ID tid) override;
  virtual void addJacobianNeighborLowerD(THREAD_ID tid) override;
  virtual void addJacobianLowerD(THREAD_ID tid) override;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) override;
  virtual void addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                                    unsigned int ivar,
                                    unsigned int jvar,
                                    const DofMap & dof_map,
                                    std::vector<dof_id_type> & dof_indices,
                                    const std::set<TagID> & tags,
                                    THREAD_ID tid);
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
  virtual void addCachedJacobian(THREAD_ID tid) override;
  /**
   * Deprecated method. Use addCachedJacobian
   */
  virtual void addCachedJacobianContributions(THREAD_ID tid) override;

  virtual void prepareShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) override;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) override;

  Assembly & assembly(THREAD_ID tid, unsigned int nl_sys_num = 0) override;
  const Assembly & assembly(THREAD_ID tid, unsigned int nl_sys_num = 0) const override;

  // Geom Search /////
  virtual void updateGeomSearch(
      GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) override;
  virtual GeometricSearchData & geomSearchData() override { return _geometric_search_data; }

  virtual bool computingInitialResidual(unsigned int nl_sys_num = 0) const override;

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

  LineSearch * getLineSearch() override;

  const CouplingMatrix * couplingMatrix(unsigned int nl_sys_num = 0) const override;

  bool haveDisplaced() const override final { return true; }

  bool computingScalingJacobian() const override final;

  bool computingScalingResidual() const override final;

  void initialSetup() override;
  void timestepSetup() override;
  void customSetup(const ExecFlagType & exec_type) override;

  using SubProblem::haveADObjects;
  void haveADObjects(bool have_ad_objects) override;

  std::size_t numNonlinearSystems() const override;

  unsigned int currentNlSysNum() const override;

protected:
  FEProblemBase & _mproblem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  /// reference mesh
  MooseMesh & _ref_mesh;
  std::vector<std::string> _displacements;

  std::vector<std::unique_ptr<DisplacedSystem>> _displaced_nl;
  std::unique_ptr<DisplacedSystem> _displaced_aux;

  /// The nonlinear system solutions
  std::vector<const NumericVector<Number> *> _nl_solution;

  /// The auxiliary system solution
  const NumericVector<Number> * _aux_solution;

  std::vector<std::vector<std::unique_ptr<Assembly>>> _assembly;

  GeometricSearchData _geometric_search_data;

private:
  std::pair<bool, unsigned int>
  determineNonlinearSystem(const std::string & var_name,
                           bool error_if_not_found = false) const override;

  friend class UpdateDisplacedMeshThread;
  friend class Restartable;
};

inline DisplacedSystem &
DisplacedProblem::nlSys(const unsigned int sys_num)
{
  mooseAssert(sys_num < _displaced_nl.size(),
              "System number greater than the number of nonlinear systems");
  return *_displaced_nl[sys_num];
}

inline const SystemBase &
DisplacedProblem::systemBaseNonlinear(const unsigned int sys_num) const
{
  mooseAssert(sys_num < _displaced_nl.size(),
              "System number greater than the number of nonlinear systems");
  return *_displaced_nl[sys_num];
}

inline SystemBase &
DisplacedProblem::systemBaseNonlinear(const unsigned int sys_num)
{
  mooseAssert(sys_num < _displaced_nl.size(),
              "System number greater than the number of nonlinear systems");
  return *_displaced_nl[sys_num];
}
