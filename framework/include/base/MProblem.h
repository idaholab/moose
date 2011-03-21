#ifndef MPROBLEM_H
#define MPROBLEM_H

#include "SubProblem.h"
#include "SubProblemInterface.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "AssemblyData.h"
#include "GeometricSearchData.h"
#include "MaterialWarehouse.h"
#include "PostprocessorWarehouse.h"
#include "PostprocessorData.h"
#include "Output.h"
#include "Adaptivity.h"

class DisplacedProblem;

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class MProblem :
  public SubProblem
{
public:
  MProblem(MooseMesh & mesh, Problem * parent = NULL);
  virtual ~MProblem();

  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);

  virtual Order getQuadratureOrder() { return _quadrature_order; }
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

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid);

  virtual void init();
  virtual void init2();
  virtual void update();
  virtual void solve();
  virtual bool converged();

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual void copySolutionsBackwards();

  // NL /////
  void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  NonlinearSystem & getNonlinearSystem() { return _nl; }

  // Aux /////
  void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return _aux; }

  // Dirac /////
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // ICs /////
  void addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name);
  void addInitialCondition(const std::string & var_name, Real value);

  Number initialValue (const Point & p, const Parameters & parameters, const std::string & /*sys_name*/, const std::string & var_name);
  Gradient initialGradient (const Point & p, const Parameters & /*parameters*/, const std::string & /*sys_name*/, const std::string & var_name);

  void initialCondition(EquationSystems & es, const std::string & system_name);

  // Materials /////
  void addMaterial(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  virtual void updateMaterials();
  virtual void reinitMaterials(unsigned int blk_id, THREAD_ID tid);
  virtual void reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid);

  // Stabilization /////
  void addStabilizer(const std::string & stabilizer_name, const std::string & name, InputParameters parameters);

  // Postprocessors /////
  virtual void addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, Moose::PostprocessorType pps_type = Moose::PPS_TIMESTEP);
  
  /**
   * Get a reference to the value associated with the postprocessor.
   */
  Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0);
  virtual void computePostprocessors(int pps_type = Moose::PPS_TIMESTEP);
  virtual void outputPostprocessors();

  // Dampers /////
  void addDamper(std::string damper_name, const std::string & name, InputParameters parameters);
  void setupDampers();

  /**
   * Whether or not this system has dampers.
   */
  bool hasDampers() { return _has_dampers; }

  ////
  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual);
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> &  jacobian);
  virtual void computeJacobianBlock(SparseMatrix<Number> &  jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar);
  virtual Real computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update);

  // Displaced problem /////
  virtual void initDisplacedProblem(const std::vector<std::string> & displacements);

  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Output /////
  virtual Output & out() { return _out; }
  virtual void output();

  // Adaptivity /////
  Adaptivity & adaptivity() { return _adaptivity; }
  virtual void adaptMesh();
  virtual void meshChanged();

  void checkProblemIntegrity();

protected:
  NonlinearSystem _nl;
  AuxiliarySystem _aux;

  // quadrature
  Order _quadrature_order;                              /// Quadrature order required by all variables to integrated over them.
  std::vector<AssemblyData *> _asm_info;

  // Initial conditions
  std::map<std::string, InitialCondition *> _ics;

  // material properties
  std::vector<MaterialData *> _material_data;
  std::vector<MaterialData *> _bnd_material_data;

  // materials
  std::vector<MaterialWarehouse> _materials;

  // postprocessors
  std::vector<PostprocessorData> _pps_data;
  std::vector<PostprocessorWarehouse> _pps;             // pps calculated every time step
  std::vector<PostprocessorWarehouse> _pps_residual;    // pps calculated every residual evaluation
  std::vector<PostprocessorWarehouse> _pps_jacobian;    // pps calculated every jacobian evaluation
  std::vector<PostprocessorWarehouse> _pps_newtonit;    // pps calculated every newton iteration
  FormattedTable _pps_output_table;

  void computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps);

  // TODO: PPS output subsystem, and this will go away
  // postprocessor output
public:
  bool _postprocessor_screen_output;
  bool _postprocessor_csv_output;
  bool _postprocessor_ensight_output;
  bool _postprocessor_gnuplot_output;
  std::string _gnuplot_format;

protected:
  // Output system
  Output _out;

  Adaptivity _adaptivity;

  // Displaced mesh /////
  MooseMesh * _displaced_mesh;
  DisplacedProblem * _displaced_problem;
  GeometricSearchData _geometric_search_data;

  bool _reinit_displaced_elem;
  bool _reinit_displaced_face;
  bool _output_displaced;                               /// true for outputting displaced problem

  bool _has_dampers;                                    /// Whether or not this system has any Dampers associated with it.

public:
  static unsigned int _n;                               /// number of instances of MProblem (to distinguish Systems when coupling problems together)

  friend class AuxiliarySystem;
  friend class NonlinearSystem;
};

#endif /* MPROBLEM_H */
