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
#include "AssemblyData.h"
#include "GeometricSearchData.h"
#include "FEProblem.h"

// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

class SubProblem;
class MooseVariable;
class AssemblyData;

class DisplacedProblem : public SubProblem
{
public:
  DisplacedProblem(FEProblem & mproblem, MooseMesh & displaced_mesh, const std::vector<std::string> & displacements, InputParameters params);
  virtual ~DisplacedProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }
  virtual Problem * parent() { return _mproblem.parent(); }
  MooseMesh & refMesh() { return _ref_mesh; }

  DisplacedSystem & nlSys() { return _displaced_nl; }
  DisplacedSystem & auxSys() { return _displaced_aux; }

  virtual void createQRules(QuadratureType type, Order order);
  virtual void init();
  virtual void solve() {}
  virtual bool converged() { return _mproblem.converged(); }

  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  // Variables /////
  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  // Output /////
  virtual void output(bool force = false);

  // Adaptivity /////
  virtual void meshChanged();

  virtual void subdomainSetup(unsigned int /*subdomain*/, THREAD_ID /*tid*/) {}
  virtual void subdomainSetupSide(unsigned int /*subdomain*/, THREAD_ID /*tid*/) {}

  // reinit /////

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid);
  virtual void prepareAssemblyNeighbor(THREAD_ID tid);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid);
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid);

  virtual void reinitMaterials(unsigned int /*blk_id*/, THREAD_ID /*tid*/) { }
  virtual void reinitMaterialsFace(unsigned int /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/) { }

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual AsmBlock & asmBlock(THREAD_ID tid);

  virtual void computeResidual(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, NumericVector<Number> & /*residual*/) {}
  virtual void computeJacobian(NonlinearImplicitSystem & /*sys*/, const NumericVector<Number> & /*soln*/, SparseMatrix<Number> & /*jacobian*/) {}

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

  virtual AssemblyData & assembly(THREAD_ID tid) { return *_asm_info[tid]; }
  virtual QBase * & qRule(THREAD_ID tid) { return _asm_info[tid]->qRule(); }
  virtual const std::vector<Point> & points(THREAD_ID tid) { return _asm_info[tid]->qPoints(); }
  virtual const std::vector<Point> & physicalPoints(THREAD_ID tid) { return _asm_info[tid]->physicalPoints(); }
  virtual const std::vector<Real> & JxW(THREAD_ID tid) { return _asm_info[tid]->JxW(); }
  virtual QBase * & qRuleFace(THREAD_ID tid) { return _asm_info[tid]->qRuleFace(); }
  virtual const std::vector<Point> & pointsFace(THREAD_ID tid) { return _asm_info[tid]->qPointsFace(); }
  virtual const std::vector<Real> & JxWFace(THREAD_ID tid) { return _asm_info[tid]->JxWFace(); }
  virtual const Elem * & elem(THREAD_ID tid) { return _asm_info[tid]->elem(); }
  virtual unsigned int & side(THREAD_ID tid) { return _asm_info[tid]->side(); }
  virtual const Elem * & sideElem(THREAD_ID tid) { return _asm_info[tid]->sideElem(); }
  virtual const Node * & node(THREAD_ID tid) { return _asm_info[tid]->node(); }
  virtual const Node * & nodeNeighbor(THREAD_ID tid) { return _asm_info[tid]->nodeNeighbor(); }

  // Geom Search /////
  virtual void updateGeomSearch();
  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Transient /////
  virtual void copySolutionsBackwards() {}

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

  // Initial conditions /////
  virtual Number initialValue (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name);
  virtual Gradient initialGradient (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name);

  // Postprocessors /////
  virtual void computePostprocessors(ExecFlagType type = EXEC_TIMESTEP);
  virtual void outputPostprocessors(bool force = false);
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0);

  /// Will make sure that all dofs connected to elem_id are ghosted to this processor
  virtual void addGhostedElem(unsigned int elem_id);
  /// Will make sure that all necessary elements from boundary_id are ghosted to this processor
  virtual void addGhostedBoundary(unsigned int boundary_id);

protected:
  Problem & _problem;
  FEProblem & _mproblem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  MooseMesh & _ref_mesh;                               ///< reference mesh
  std::vector<std::string> _displacements;

  DisplacedSystem _displaced_nl;
  DisplacedSystem _displaced_aux;

  const NumericVector<Number> * _nl_solution;
  const NumericVector<Number> * _aux_solution;

  std::vector<AsmBlock *> _asm_block;                   ///<
  std::vector<AssemblyData *> _asm_info;

  GeometricSearchData _geometric_search_data;

  ExodusOutput _ex;


  friend class UpdateDisplacedMeshThread;
};

#endif /* DISPLACEDPROBLEM_H */
