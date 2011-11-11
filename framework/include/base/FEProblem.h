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

#ifndef FEPROBLEM_H
#define FEPROBLEM_H

#include "SubProblem.h"
#include "SubProblemInterface.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "AssemblyData.h"
#include "GeometricSearchData.h"
#include "MaterialWarehouse.h"
#include "MaterialPropertyStorage.h"
#include "PostprocessorWarehouse.h"
#include "PostprocessorData.h"
#include "Output.h"
#include "Adaptivity.h"

class DisplacedProblem;
class OutputProblem;

class FEProblem;

template<>
InputParameters validParams<FEProblem>();

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class FEProblem :
  public SubProblem
{
public:
  FEProblem(const std::string & name, InputParameters parameters);
  virtual ~FEProblem();

  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);

  virtual void createQRules(QuadratureType type, Order order);
  virtual Order getQuadratureOrder() { return _quadrature_order; }
  virtual AsmBlock & asmBlock(THREAD_ID tid);
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

  /**
   * Returns a list of all the variables in the problem (both from the NL and Aux systems.
   */
  std::vector<std::string> getVariableNames();


  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid);

  virtual void prepareAssembly(THREAD_ID tid);

  virtual void addGhostedElem(unsigned int elem_id);
  virtual void addGhostedBoundary(unsigned int boundary_id);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid);

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid);
  virtual void subdomainSetupSide(unsigned int subdomain, THREAD_ID tid);

  virtual void init();
  virtual void init2();
  virtual void solve();
  virtual bool converged();
  virtual unsigned int nNonlinearIterations() { return _nl.nNonlinearIterations(); }
  virtual unsigned int nLinearIterations() { return _nl.nLinearIterations(); }
  virtual Real finalNonlinearResidual() { return _nl.finalNonlinearResidual(); }

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual void copySolutionsBackwards();
  // Update backward time solution vectors
  virtual void copyOldSolutions();

  // NL /////
  NonlinearSystem & getNonlinearSystem() { return _nl; }
  void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);
  void addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  // Aux /////
  void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return _aux; }

  // Dirac /////
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // DG /////
  void addDGKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // ICs /////
  void addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters, std::string var_name);
  void addInitialCondition(const std::string & var_name, Real value);

  Number initialValue (const Point & p, const Parameters & parameters, const std::string & /*sys_name*/, const std::string & var_name);
  Gradient initialGradient (const Point & p, const Parameters & /*parameters*/, const std::string & /*sys_name*/, const std::string & var_name);

  void projectSolution();

  // Materials /////
  void addMaterial(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  virtual const std::vector<Material*> & getMaterials(unsigned int block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getFaceMaterials(unsigned int block_id, THREAD_ID tid);
  virtual void updateMaterials();
  virtual void reinitMaterials(unsigned int blk_id, THREAD_ID tid);
  virtual void reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid);
  virtual void reinitMaterialsNeighbor(unsigned int blk_id, unsigned int side, THREAD_ID tid);

  // Postprocessors /////
  virtual void addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters, ExecFlagType type = EXEC_TIMESTEP);

  /**
   * Get a reference to the value associated with the postprocessor.
   */
  Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0);
  virtual void computePostprocessors(ExecFlagType type = EXEC_TIMESTEP);
  virtual void computeAuxiliaryKernels(ExecFlagType type = EXEC_RESIDUAL);
  virtual void outputPostprocessors(bool force = false);

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

  // Displaced problem /////
  virtual void initDisplacedProblem(MooseMesh * displaced_mesh, const std::vector<std::string> & displacements);
  virtual DisplacedProblem * getDisplacedProblem() { return _displaced_problem; }

  virtual void updateGeomSearch();

  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Output /////
  virtual Output & out() { return _out; }
  virtual void output(bool force = false);
  virtual void outputDisplaced(bool state = true) { _output_displaced = state; }
  OutputProblem & getOutputProblem(unsigned int refinements);
  void setMaxPPSRowsScreen(unsigned int n) { _pps_output_table_max_rows = n; }

  // Restart //////
  virtual void setRestartFile(const std::string & file_name) { _restart = true; _restart_file_name = file_name; }
  virtual void restartFromFile();

#ifdef LIBMESH_ENABLE_AMR
  // Adaptivity /////
  Adaptivity & adaptivity() { return _adaptivity; }
  virtual void adaptMesh();
  virtual void setUniformRefineLevel(unsigned int level) { _uniform_refine_level = level; }
#endif //LIBMESH_ENABLE_AMR
  virtual void meshChanged();

  void checkProblemIntegrity();

  void serializeSolution();

  inline void setEarlyPerfLogPrint(bool val) { _output_setup_log_early = val; }

  // debugging iface /////

  /// Set the number of top residual to be printed out (0 = no output)
  void setDebugTopResiduals(unsigned int n) { _dbg_top_residuals = n; }


protected:
  NonlinearSystem _nl;
  AuxiliarySystem _aux;

  // quadrature
  Order _quadrature_order;                              ///< Quadrature order required by all variables to integrated over them.
  std::vector<AssemblyData *> _asm_info;

  // Initial conditions
  std::map<std::string, InitialCondition *> _ics;

  // material properties
  MaterialPropertyStorage _material_props;
  MaterialPropertyStorage _bnd_material_props;

  std::vector<MaterialData *> _material_data;
  std::vector<MaterialData *> _bnd_material_data;
  std::vector<MaterialData *> _neighbor_material_data;

  // materials
  std::vector<MaterialWarehouse> _materials;

  // postprocessors
  std::vector<PostprocessorData> _pps_data;
  ExecStore<PostprocessorWarehouse> _pps;
  FormattedTable _pps_output_table;
  unsigned int _pps_output_table_max_rows;

  void computePostprocessorsInternal(std::vector<PostprocessorWarehouse> & pps);

  // TODO: PPS output subsystem, and this will go away
  // postprocessor output
public:
  bool _postprocessor_screen_output;
  bool _postprocessor_csv_output;
  bool _postprocessor_ensight_output;
  bool _postprocessor_gnuplot_output;
  std::string _gnuplot_format;

  ExodusII_IO * _ex_reader; // indirect ptr to ex_reader used for copying nodal values

protected:
  void checkPPSs();

  /**
   * Add postprocessor values to the output table
   * @param type type of PPS to add to the table
   */
  void addPPSValuesToTable(ExecFlagType type);

  // Output system
  Output _out;
  OutputProblem * _out_problem;

#ifdef LIBMESH_ENABLE_AMR
  Adaptivity _adaptivity;
  unsigned int _uniform_refine_level;
#endif

  // Displaced mesh /////
  MooseMesh * _displaced_mesh;
  DisplacedProblem * _displaced_problem;
  GeometricSearchData _geometric_search_data;

  bool _reinit_displaced_elem;
  bool _reinit_displaced_face;
  bool _output_displaced;                               ///< true for outputting displaced problem
  bool _input_file_saved;                               ///< whether input file has been written

  bool _has_dampers;                                    ///< Whether or not this system has any Dampers associated with it.

  bool _has_constraints;                                /// Whether or not this system has any Constraints.

  bool _restart;                                        ///< true if restarting from a file, otherwise false
  std::string _restart_file_name;                       ///< name of the file that we restart from

  std::set<unsigned int> _ghosted_elems;                ///< Elements that should have Dofs ghosted to the local processor

//  PerfLog _solve_only_perf_log;                         ///< Only times the solve
  bool _output_setup_log_early;                         ///< Determines if the setup log is printed before the first time step

  // DEBUGGING capabilities
  unsigned int _dbg_top_residuals;                      ///< Number of top residual to print out

public:
  static unsigned int _n;                               ///< number of instances of FEProblem (to distinguish Systems when coupling problems together)

  friend class AuxiliarySystem;
  friend class NonlinearSystem;
};

#endif /* FEPROBLEM_H */
