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
#include "MooseMesh.h"
#include "ExodusOutput.h"
#include "DisplacedSystem.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "FEProblem.h"

// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

class SubProblem;
class MooseVariable;
class AssemblyData;
class DisplacedProblem;

template<>
InputParameters validParams<DisplacedProblem>();


class DisplacedProblem : public SubProblem
{
public:
  DisplacedProblem(FEProblem & mproblem, MooseMesh & displaced_mesh, InputParameters params);
  virtual ~DisplacedProblem();

  virtual MooseMesh & mesh() { return _mesh; }
  MooseMesh & refMesh() { return _ref_mesh; }

  DisplacedSystem & nlSys() { return _displaced_nl; }
  DisplacedSystem & auxSys() { return _displaced_aux; }

  virtual void createQRules(QuadratureType type, Order order);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache);

  virtual void init();
  virtual void solve() {}
  virtual bool converged() { return _mproblem.converged(); }

  virtual void syncSolutions(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);
  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  // Variables /////
  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual bool hasScalarVariable(const std::string & var_name);
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);
  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains = NULL);
  virtual void addScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  virtual void addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);

  // Output /////
  virtual void output(bool force = false);

  // Adaptivity /////
  virtual void initAdaptivity();
  virtual void meshChanged();

  virtual void subdomainSetup(SubdomainID /*subdomain*/, THREAD_ID /*tid*/) {}
  virtual void subdomainSetupSide(SubdomainID /*subdomain*/, THREAD_ID /*tid*/) {}

  // reinit /////

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid);
  virtual void prepareAssemblyNeighbor(THREAD_ID tid);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid);
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid);
  virtual void reinitScalars(THREAD_ID tid);

  virtual void reinitMaterials(SubdomainID /*blk_id*/, THREAD_ID /*tid*/) { }
  virtual void reinitMaterialsFace(SubdomainID /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/) { }
  virtual void reinitMaterialsBoundary(BoundaryID /*bnd_id*/, THREAD_ID /*tid*/) { }

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, NumericVector<Number> & /*residual*/) {}
  virtual void computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, SparseMatrix<Number> & /*jacobian*/) {}
  virtual void computeBounds(NonlinearImplicitSystem & /*sys*/, NumericVector<Number> & /*lower*/, NumericVector<Number> & /*upper*/){}

  virtual void addResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void cacheResidual(THREAD_ID tid);
  virtual void cacheResidualNeighbor(THREAD_ID tid);
  virtual void addCachedResidual(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid);

  virtual void cacheJacobian(THREAD_ID tid);
  virtual void cacheJacobianNeighbor(THREAD_ID tid);
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);

  virtual void prepareShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid);

  virtual Assembly & assembly(THREAD_ID tid) { return *_assembly[tid]; }
  virtual const Moose::CoordinateSystemType & coordSystem(THREAD_ID tid) { return _assembly[tid]->coordSystem(); }
  virtual QBase * & qRule(THREAD_ID tid) { return _assembly[tid]->qRule(); }
  virtual const MooseArray<Point> & points(THREAD_ID tid) { return _assembly[tid]->qPoints(); }
  virtual const MooseArray<Point> & physicalPoints(THREAD_ID tid) { return _assembly[tid]->physicalPoints(); }
  virtual const MooseArray<Real> & JxW(THREAD_ID tid) { return _assembly[tid]->JxW(); }
  virtual const Real & elemVolume(THREAD_ID tid) { return _assembly[tid]->elemVolume(); }
  virtual const MooseArray<Real> & coords(THREAD_ID tid) { return _assembly[tid]->coordTransformation(); } // have to use coord transformation from undisplaced problem
  virtual QBase * & qRuleFace(THREAD_ID tid) { return _assembly[tid]->qRuleFace(); }
  virtual const MooseArray<Point> & pointsFace(THREAD_ID tid) { return _assembly[tid]->qPointsFace(); }
  virtual const MooseArray<Real> & JxWFace(THREAD_ID tid) { return _assembly[tid]->JxWFace(); }
  virtual const Real & sideElemVolume(THREAD_ID tid) { return _assembly[tid]->sideElemVolume(); }
  virtual const Elem * & elem(THREAD_ID tid) { return _assembly[tid]->elem(); }
  virtual unsigned int & side(THREAD_ID tid) { return _assembly[tid]->side(); }
  virtual const Elem * & sideElem(THREAD_ID tid) { return _assembly[tid]->sideElem(); }
  virtual const Node * & node(THREAD_ID tid) { return _assembly[tid]->node(); }
  virtual const Node * & nodeNeighbor(THREAD_ID tid) { return _assembly[tid]->nodeNeighbor(); }

  // Geom Search /////
  virtual void updateGeomSearch();
  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Transient /////
  virtual void copySolutionsBackwards() {}

  virtual bool computingInitialResidual() { return _mproblem.computingInitialResidual(); }

  virtual void onTimestepBegin() {}
  virtual void onTimestepEnd() {}

  virtual Real & time() { return _mproblem.time(); }
  virtual int & timeStep() { return _mproblem.timeStep(); }
  virtual Real & dt() { return _mproblem.dt(); }
  virtual Real & dtOld() { return _mproblem.dtOld(); }

  virtual void transient(bool trans) { _mproblem.transient(trans); }
  virtual bool isTransient() { return _mproblem.isTransient(); }

  virtual std::vector<Real> & timeWeights() { return _mproblem.timeWeights(); }

  virtual Order getQuadratureOrder() { return _mproblem.getQuadratureOrder(); }

  // Postprocessors /////
  virtual void computePostprocessors(ExecFlagType type = EXEC_TIMESTEP);
  virtual void outputPostprocessors(bool force = false);
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0);
  virtual void outputPps(const FormattedTable & table);

  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  /**
   * Set which variables will be written in output files
   * @param output_variables The list of variable names to write in the output files
   */
  virtual void setOutputVariables(std::vector<std::string> output_variables);

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(unsigned int elem_id);
  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   * @param boundary_id Boundary ID
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id);

protected:
  FEProblem & _mproblem;
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

  ExodusOutput _ex;
  bool _seq;


  friend class UpdateDisplacedMeshThread;
};

#endif /* DISPLACEDPROBLEM_H */
