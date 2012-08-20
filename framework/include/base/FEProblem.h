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

#include "Moose.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "MaterialWarehouse.h"
#include "MaterialPropertyStorage.h"
#include "PostprocessorWarehouse.h"
#include "UserObjectWarehouse.h"
#include "PostprocessorData.h"
#include "Output.h"
#include "Adaptivity.h"
#include "Resurrector.h"
#include "IndicatorWarehouse.h"
#include "MarkerWarehouse.h"
#include "TimeScheme.h"
class DisplacedProblem;
class OutputProblem;
class TimeScheme;

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

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }

  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid);
  virtual void setCoordSystem(const std::vector<SubdomainName> & blocks, const std::vector<std::string> & coord_sys);

  /**
   * Set the coupling between variables
   * TODO: allow user-defined coupling
   * @param type Type of coupling
   */
  void setCoupling(Moose::CouplingType type);

  Moose::CouplingType coupling() { return _coupling; }

  /**
   * Set custom coupling matrix
   * @param cm coupling matrix to be set
   */
  void setCouplingMatrix(CouplingMatrix * cm);
  CouplingMatrix * & couplingMatrix() { return _cm; }

  std::vector<std::pair<unsigned int, unsigned int> > & couplingEntries(THREAD_ID tid) { return _assembly[tid]->couplingEntries(); }

#ifdef LIBMESH_HAVE_PETSC
  void storePetscOptions(const std::vector<std::string> & petsc_options,
                         const std::vector<std::string> & petsc_options_inames,
                         const std::vector<std::string> & petsc_options_values);
#endif

  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual bool hasScalarVariable(const std::string & var_name);
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);

  virtual void createQRules(QuadratureType type, Order order);
  virtual Order getQuadratureOrder() { return _quadrature_order; }
  virtual Assembly & assembly(THREAD_ID tid) { return *_assembly[tid]; }
  virtual const Moose::CoordinateSystemType & coordSystem(THREAD_ID tid) { return _assembly[tid]->coordSystem(); }
  virtual QBase * & qRule(THREAD_ID tid) { return _assembly[tid]->qRule(); }
  virtual const MooseArray<Point> & points(THREAD_ID tid) { return _assembly[tid]->qPoints(); }
  virtual const MooseArray<Point> & physicalPoints(THREAD_ID tid) { return _assembly[tid]->physicalPoints(); }
  virtual const MooseArray<Real> & JxW(THREAD_ID tid) { return _assembly[tid]->JxW(); }
  virtual const Real & elemVolume(THREAD_ID tid) { return _assembly[tid]->elemVolume(); }
  virtual const MooseArray<Real> & coords(THREAD_ID tid) { return _assembly[tid]->coordTransformation(); }
  virtual QBase * & qRuleFace(THREAD_ID tid) { return _assembly[tid]->qRuleFace(); }
  virtual const MooseArray<Point> & pointsFace(THREAD_ID tid) { return _assembly[tid]->qPointsFace(); }
  virtual const MooseArray<Real> & JxWFace(THREAD_ID tid) { return _assembly[tid]->JxWFace(); }
  virtual const Real & sideElemVolume(THREAD_ID tid) { return _assembly[tid]->sideElemVolume(); }

  virtual const Elem * & elem(THREAD_ID tid) { return _assembly[tid]->elem(); }
  virtual unsigned int & side(THREAD_ID tid) { return _assembly[tid]->side(); }
  virtual const Elem * & sideElem(THREAD_ID tid) { return _assembly[tid]->sideElem(); }
  virtual const Node * & node(THREAD_ID tid) { return _assembly[tid]->node(); }
  virtual const Node * & nodeNeighbor(THREAD_ID tid) { return _assembly[tid]->nodeNeighbor(); }

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
  virtual void addGhostedBoundary(BoundaryID boundary_id);

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

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid);
  virtual void subdomainSetupSide(SubdomainID subdomain, THREAD_ID tid);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache);

  virtual void init();
  virtual void init2();
  virtual void solve();
  virtual bool converged();
  virtual unsigned int nNonlinearIterations() { return _nl.nNonlinearIterations(); }
  virtual unsigned int nLinearIterations() { return _nl.nLinearIterations(); }
  virtual Real finalNonlinearResidual() { return _nl.finalNonlinearResidual(); }

  virtual bool computingInitialResidual() { return _nl.computingInitialResidual(); }

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual Real & time() { return _time; }
  virtual Real & timeOld() { return _time_old; }
  virtual int & timeStep() { return _t_step; }
  virtual Real & dt() { return _dt; }
  virtual Real & dtOld() { return _dt_old; }
  virtual TimeScheme * getTimeScheme(){ return _nl.getTimeScheme();}

  virtual void transient(bool trans) { _transient = trans; }
  virtual bool isTransient() { return _transient; }

  virtual std::vector<Real> & timeWeights() { return _time_weights; }

  virtual void copySolutionsBackwards();
  // Update backward time solution vectors
  virtual void copyOldSolutions();
  virtual void restoreSolutions();

  virtual const std::vector<MooseObject *> & getObjectsByName(const std::string & name, THREAD_ID tid);

  // Function /////
  virtual void addFunction(std::string type, const std::string & name, InputParameters parameters);
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  // NL /////
  NonlinearSystem & getNonlinearSystem() { return _nl; }
  void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL);
  void addScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);
  void addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters);

  // Aux /////
  void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains = NULL);
  void addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  void addAuxKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addAuxBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  AuxiliarySystem & getAuxiliarySystem() { return _aux; }

  // Dirac /////
  void addDiracKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // DG /////
  void addDGKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);

  // IC /////
  void addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters);

  void projectSolution();

  // Materials /////
  void addMaterial(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  /**
   * Get list of materials with specified name
   * @param name The name of the material
   * @param tid Thread ID
   * @return The list of materials with the name 'name'
   */
  virtual const std::vector<Material*> & getMaterialsByName(const std::string & name, THREAD_ID tid);
  virtual const std::vector<Material*> & getMaterials(SubdomainID block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getFaceMaterials(SubdomainID block_id, THREAD_ID tid);
  virtual const std::vector<Material*> & getBndMaterials(BoundaryID block_id, THREAD_ID tid);
  virtual void updateMaterials();
  virtual void reinitMaterials(SubdomainID blk_id, THREAD_ID tid);
  virtual void reinitMaterialsFace(SubdomainID blk_id, unsigned int side, THREAD_ID tid);
  virtual void reinitMaterialsNeighbor(SubdomainID blk_id, unsigned int side, THREAD_ID tid);
  virtual void reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid);

  // Postprocessors /////
  virtual void addPostprocessor(std::string pp_name, const std::string & name, InputParameters parameters);


  // UserObjects /////
  virtual void addUserObject(std::string user_object_name, const std::string & name, InputParameters parameters);

  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @param tid The thread ID
   * @return Const reference to the user object
   */
  template <class T>
  const T & getUserObject(const std::string & name)
  {
    UserObject * user_object = NULL;

    ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL };
    for (unsigned int i = 0; i < LENGTHOF(types) && !user_object; i++)
      user_object = _user_objects(types[i])[0].getUserObjectByName(name);

    if(!user_object)
      mooseError("Unable to find user object with name '" + name + "'");

    return dynamic_cast<const T &>(*user_object);
  }

  /**
   * Check if there if a user object of given name
   * @param name The name of the user object being checked for
   * @param tid  The thread ID
   * @return true if the user object exists, false otherwise
   */
  bool hasUserObject(const std::string & name);

  /**
   * Get a reference to the value associated with the postprocessor.
   */
  Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0);
  virtual void computeUserObjects(ExecFlagType type = EXEC_TIMESTEP, UserObjectWarehouse::GROUP group = UserObjectWarehouse::ALL);
  virtual void computeAuxiliaryKernels(ExecFlagType type = EXEC_RESIDUAL);
  virtual void outputPostprocessors(bool force = false);

  // Dampers /////
  void addDamper(std::string damper_name, const std::string & name, InputParameters parameters);
  void setupDampers();

  /**
   * Whether or not this system has dampers.
   */
  bool hasDampers() { return _has_dampers; }

  // Indicators /////
  void addIndicator(std::string indicator_name, const std::string & name, InputParameters parameters);

  // Markers //////
  void addMarker(std::string marker_name, const std::string & name, InputParameters parameters);

  /// Evaluates transient residual G in canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitResidual(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, NumericVector<Number>& residual);

  /// Evaluates transient Jacobian J_a = dG/dU + a*dG/dUdot from canonical semidiscrete form G(t,U,Udot) = F(t,U)
  void computeTransientImplicitJacobian(Real time, const NumericVector<Number>& u, const NumericVector<Number>& udot, Real shift, SparseMatrix<Number> &jacobian);

  ////
  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual );
  virtual void computeResidualType(const NumericVector<Number> & soln, NumericVector<Number> & residual, Moose::KernelType type = Moose::KT_ALL );
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> &  jacobian);
  virtual void computeJacobianBlock(SparseMatrix<Number> &  jacobian, libMesh::System & precond_system, unsigned int ivar, unsigned int jvar);
  virtual Real computeDamping(const NumericVector<Number>& soln, const NumericVector<Number>& update);
  virtual void computeBounds(NonlinearImplicitSystem & sys, NumericVector<Number> & lower, NumericVector<Number> & upper);

  virtual void computeIndicatorsAndMarkers();

  virtual void addResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void addResidualScalar(NumericVector<Number> & residual, THREAD_ID tid = 0);

  virtual void cacheResidual(THREAD_ID tid);
  virtual void cacheResidualNeighbor(THREAD_ID tid);
  virtual void addCachedResidual(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid);
  virtual void addJacobianScalar(SparseMatrix<Number> & jacobian, THREAD_ID tid = 0);

  virtual void cacheJacobian(THREAD_ID tid);
  virtual void cacheJacobianNeighbor(THREAD_ID tid);
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);

  virtual void prepareShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid);

  // Displaced problem /////
  virtual void initDisplacedProblem(MooseMesh * displaced_mesh, InputParameters params);
  virtual DisplacedProblem * & getDisplacedProblem() { return _displaced_problem; }

  virtual void updateGeomSearch();

  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  // Output /////
  virtual Output & out() { return _out; }
  virtual void output(bool force = false);
  virtual void outputDisplaced(bool state = true) { _output_displaced = state; }
  virtual void outputSolutionHistory(bool state = true) { _output_solution_history = state; }
  /**
   * Set which variables will be written in ouput files
   * @param output_variables The list of variable names to write in the ouput files
   */
  virtual void setOutputVariables(std::vector<std::string> output_variables);
  OutputProblem & getOutputProblem(unsigned int refinements);
  void setMaxPPSRowsScreen(unsigned int n) { _pps_output_table_max_rows = n; }

  // Restart //////

  /**
   * Set a file we will restart from
   * @param file_name The file name we will restart from
   */
  virtual void setRestartFile(const std::string & file_name);

  /**
   * Set the number of restart files to save
   * @param num_files Number of files to keep around
   */
  virtual void setNumRestartFiles(unsigned int num_files);

  /**
   * Was this subproblem initialized from a restart file
   * @return true if we restarted form a file, otherwise false
   */
  virtual bool isRestarting();

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

  /**
   * Set the number of top residual to be printed out (0 = no output)
   */
  void setDebugTopResiduals(unsigned int n) { _dbg_top_residuals = n; }


protected:
  MooseMesh & _mesh;
  EquationSystems _eq;

  bool _transient;
  Real & _time;
  Real & _time_old;
  int & _t_step;
  Real & _dt;
  Real _dt_old;

  std::vector<Real> _time_weights;

  /// Objects by names, indexing: [thread][name]->array of moose objects with name 'name'
  std::vector<std::map<std::string, std::vector<MooseObject *> > > _objects_by_name;

  NonlinearSystem _nl;
  AuxiliarySystem _aux;

  Moose::CouplingType _coupling;                        ///< Type of variable coupling
  CouplingMatrix * _cm;                                 ///< Coupling matrix for variables. It is diagonal, since we do only block diagonal preconditioning.

  // quadrature
  Order _quadrature_order;                              ///< Quadrature order required by all variables to integrated over them.
  std::vector<Assembly *> _assembly;

  /// functions
  std::vector<std::map<std::string, Function *> > _functions;

  // material properties
  MaterialPropertyStorage _material_props;
  MaterialPropertyStorage _bnd_material_props;

  std::vector<MaterialData *> _material_data;
  std::vector<MaterialData *> _bnd_material_data;
  std::vector<MaterialData *> _neighbor_material_data;

  // materials
  std::vector<MaterialWarehouse> _materials;

  // indicators
  std::vector<IndicatorWarehouse> _indicators;

  // markers
  std::vector<MarkerWarehouse> _markers;

  // postprocessors
  std::vector<PostprocessorData> _pps_data;
  ExecStore<PostprocessorWarehouse> _pps;

  // user objects
  ExecStore<UserObjectWarehouse> _user_objects;

  /// Table with postprocessors that will go into files
  FormattedTable _pps_output_table_file;
  /// Table with postprocessors that will go on screen
  FormattedTable _pps_output_table_screen;
  unsigned int _pps_output_table_max_rows;

  void computeUserObjectsInternal(std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group);

  // TODO: PPS output subsystem, and this will go away
  // postprocessor output
public:
  bool _postprocessor_screen_output;
  bool _postprocessor_csv_output;
  bool _postprocessor_gnuplot_output;
  std::string _gnuplot_format;

  /// indirect ptr to ex_reader used for copying nodal values
  ExodusII_IO * _ex_reader;

protected:
  void checkUserObjects();

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
  /// true for outputting displaced problem
  bool _output_displaced;
  /// true for outputting solution history
  bool _output_solution_history;
  /// whether input file has been written
  bool _input_file_saved;

  /// Whether or not this system has any Dampers associated with it.
  bool _has_dampers;

  /// Whether or not this system has any Constraints.
  bool _has_constraints;

  /// Object responsible for restart (read/write)
  Resurrector * _resurrector;

  /// Elements that should have Dofs ghosted to the local processor
  std::set<unsigned int> _ghosted_elems;

//  PerfLog _solve_only_perf_log;                         ///< Only times the solve
  /// Determines if the setup log is printed before the first time step
  bool _output_setup_log_early;

  // DEBUGGING capabilities

  /// Number of top residual to print out
  unsigned int _dbg_top_residuals;

public:
  /// number of instances of FEProblem (to distinguish Systems when coupling problems together)
  static unsigned int _n;

  friend class AuxiliarySystem;
  friend class NonlinearSystem;
  friend class Resurrector;
};

#endif /* FEPROBLEM_H */

