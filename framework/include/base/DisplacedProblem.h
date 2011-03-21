#ifndef DISPLACEDPROBLEM_H_
#define DISPLACEDPROBLEM_H_

#include "Mesh.h"
#include "ExodusOutput.h"
// libMesh
#include "equation_systems.h"
#include "explicit_system.h"
#include "numeric_vector.h"

namespace Moose
{

class Problem;

class DisplacedProblem
{
public:
  DisplacedProblem(Mesh & displaced_mesh, Mesh & mesh, const std::vector<std::string> & displacements);
  virtual ~DisplacedProblem();

  Mesh & refMesh() { return _ref_mesh; }

  virtual void init();

  /**
   * Serialize the solution
   */
  virtual void serializeSolution(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);
  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  virtual void addVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);

  // Output /////
  virtual void output(Real time);

protected:
  Mesh & _mesh;
  EquationSystems _eq;
  Mesh & _ref_mesh;                               /// reference mesh
  std::vector<std::string> _displacements;

  libMesh::ExplicitSystem & _nl_sys;
  libMesh::ExplicitSystem & _aux_sys;

  NumericVector<Number> * _nl_solution;
  NumericVector<Number> * _aux_solution;

//  std::vector<ElementData *> _element_data_displaced;
//  std::vector<FaceData *> _face_data_displaced;
//  std::vector<DiracKernelData *> _dirac_kernel_data;
//  DiracKernelInfo _dirac_kernel_info_displaced;
//  GeometricSearchData _geometric_search_data_displaced;

  ExodusOutput _ex;

  friend class UpdateDisplacedMeshThread;
};

}

#endif /* DISPLACEDPROBLEM_H_ */
