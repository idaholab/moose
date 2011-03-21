#ifndef DISPLACEDPROBLEM_H_
#define DISPLACEDPROBLEM_H_

#include "Mesh.h"
#include "ExodusOutput.h"
#include "ExplicitSystem.h"
#include "GeometricSearchData.h"
// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

namespace Moose
{

class Problem;
class SubProblem;
class Variable;

class DisplacedProblem
{
public:
  DisplacedProblem(SubProblem & problem, Mesh & displaced_mesh, Mesh & mesh, const std::vector<std::string> & displacements);
  virtual ~DisplacedProblem();

  Mesh & refMesh() { return _ref_mesh; }

  ExplicitSystem & nlSys() { return _nl; }
  ExplicitSystem & auxSys() { return _aux; }

  virtual void init();

  /**
   * Serialize the solution
   */
  virtual void serializeSolution(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);
  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  // Variables /////
  virtual Variable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  // Reinit /////
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);

  // Output /////
  virtual void output(Real time);

protected:
  SubProblem & _problem;
  Mesh & _mesh;
  EquationSystems _eq;
  Mesh & _ref_mesh;                               /// reference mesh
  std::vector<std::string> _displacements;

  ExplicitSystem _nl;
  ExplicitSystem _aux;

  NumericVector<Number> * _nl_solution;
  NumericVector<Number> * _aux_solution;

//  std::vector<ElementData *> _element_data_displaced;
//  std::vector<FaceData *> _face_data_displaced;

//  std::vector<DiracKernelData *> _dirac_kernel_data;
//  DiracKernelInfo _dirac_kernel_info_displaced;
  GeometricSearchData _geometric_search_data;

  ExodusOutput _ex;

  friend class UpdateDisplacedMeshThread;
};

}

#endif /* DISPLACEDPROBLEM_H_ */
