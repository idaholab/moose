#ifndef DISPLACEDPROBLEM_H
#define DISPLACEDPROBLEM_H

#include "SubProblem.h"
#include "MooseMesh.h"
#include "ExodusOutput.h"
#include "DisplacedSystem.h"
#include "AssemblyData.h"
#include "GeometricSearchData.h"
// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

class SubProblemInterface;
class MooseVariable;
class AssemblyData;

class DisplacedProblem :
  public SubProblemInterface
{
public:
  DisplacedProblem(SubProblem & problem, MooseMesh & displaced_mesh, const std::vector<std::string> & displacements);
  virtual ~DisplacedProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }
  virtual Problem * parent() { return _subproblem.parent(); }
  MooseMesh & refMesh() { return _ref_mesh; }

  DisplacedSystem & nlSys() { return _nl; }
  DisplacedSystem & auxSys() { return _aux; }

  virtual void init();

  /**
   * Serialize the solution
   */
  virtual void serializeSolution(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);
  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  // Variables /////
  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  // Output /////
  virtual void output();

  // reinit /////

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual AssemblyData & assembly(THREAD_ID tid) { return *_asm_info[tid]; }
  virtual QBase * & qRule(THREAD_ID tid) { return _asm_info[tid]->qRule(); }
  virtual const std::vector<Point> & points(THREAD_ID tid) { return _asm_info[tid]->qPoints(); }
  virtual const std::vector<Real> & JxW(THREAD_ID tid) { return _asm_info[tid]->JxW(); }
  virtual QBase * & qRuleFace(THREAD_ID tid) { return _asm_info[tid]->qRuleFace(); }
  virtual const std::vector<Point> & pointsFace(THREAD_ID tid) { return _asm_info[tid]->qPointsFace(); }
  virtual const std::vector<Real> & JxWFace(THREAD_ID tid) { return _asm_info[tid]->JxWFace(); }
  virtual const Elem * & elem(THREAD_ID tid) { return _asm_info[tid]->elem(); }
  virtual unsigned int & side(THREAD_ID tid) { return _asm_info[tid]->side(); }
  virtual const Elem * & sideElem(THREAD_ID tid) { return _asm_info[tid]->sideElem(); }
  virtual const Node * & node(THREAD_ID tid) { return _asm_info[tid]->node(); }

  // Geom Search /////
  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

protected:
  Problem & _problem;
  SubProblem & _subproblem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  MooseMesh & _ref_mesh;                               /// reference mesh
  std::vector<std::string> _displacements;

  DisplacedSystem _nl;
  DisplacedSystem _aux;

  NumericVector<Number> * _nl_solution;
  NumericVector<Number> * _aux_solution;

  std::vector<AssemblyData *> _asm_info;

  GeometricSearchData _geometric_search_data;

  ExodusOutput _ex;


  friend class UpdateDisplacedMeshThread;
};

#endif /* DISPLACEDPROBLEM_H_ */
