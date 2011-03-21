#ifndef DISPLACEDPROBLEM_H
#define DISPLACEDPROBLEM_H

#include "ProblemInterface.h"
#include "MooseMesh.h"
#include "ExodusOutput.h"
#include "DisplacedSystem.h"
#include "AssemblyData.h"
#include "GeometricSearchData.h"
// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

class Problem;
class SubProblem;
class MooseVariable;
class AssemblyData;

class DisplacedProblem :
  public ProblemInterface
{
public:
  DisplacedProblem(SubProblem & problem, MooseMesh & displaced_mesh, MooseMesh & mesh, const std::vector<std::string> & displacements);
  virtual ~DisplacedProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }
  MooseMesh & refMesh() { return _ref_mesh; }
  virtual Problem * parent() { return NULL; }           // for future
  virtual AssemblyData & assembly(THREAD_ID tid) { return *_asm_info[tid]; }

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
  virtual void output(Real time);

  // reinit /////

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid);

  // Transient /////
  virtual bool transient();

protected:
  SubProblem & _problem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  MooseMesh & _ref_mesh;                               /// reference mesh
  std::vector<std::string> _displacements;

  DisplacedSystem _nl;
  DisplacedSystem _aux;

  NumericVector<Number> * _nl_solution;
  NumericVector<Number> * _aux_solution;

  std::vector<AssemblyData *> _asm_info;

//  std::vector<DiracKernelData *> _dirac_kernel_data;
//  DiracKernelInfo _dirac_kernel_info_displaced;
  GeometricSearchData _geometric_search_data;

  ExodusOutput _ex;

  friend class UpdateDisplacedMeshThread;
};

#endif /* DISPLACEDPROBLEM_H_ */
