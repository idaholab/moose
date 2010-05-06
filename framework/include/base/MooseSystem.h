#ifndef MOOSESYSTEM_H
#define MOOSESYSTEM_H

#include <vector>
#include <list>

//MOOSE includes
#include "Moose.h"  // for THREAD_ID
#include "KernelHolder.h"
#include "BCHolder.h"
#include "AuxHolder.h"
#include "MaterialHolder.h"
#include "StabilizerHolder.h"
#include "ICHolder.h"

//libmesh includes
#include "transient_system.h"
#include "dof_map.h"
#include "mesh_base.h"

//Forward Declarations
class EquationSystems;
class ElementData;
class MeshBase;
class FaceData;
class AuxData;
class Material;
template<class T> class NumericVector;

/**
 * This class represents one full coupled system of nonlinear equations including any
 * explicit (Aux) equations.
 *
 * You can have multiple MOOSE systems... and (one day) couple them together).
 */
class MooseSystem
{
public:
  MooseSystem();
  MooseSystem(Mesh &mesh);
  ~MooseSystem();

  /**
   * Data Accessors for the various FE datastructures indexed by thread
   */
  ElementData * getElementData(THREAD_ID tid);
  FaceData * getFaceData(THREAD_ID tid);
  AuxData * getAuxData(THREAD_ID tid);

  /**
   * Initialize the system
   */
  void init();

  /**
   * Initialize the Mesh for this MooseSystem and return a reference
   */
  Mesh * initMesh(unsigned int dim);

  /**
   * Returns a writable reference to the mesh held wihin this MooseSystem
   */
  Mesh * getMesh();

  inline unsigned int getDim() { return _dim; }
  
  /**
   * Initialize the EquationSystems object and add both the nonlinear and auxiliary systems
   * to that object for this MooseSystem
   */
  EquationSystems * initEquationSystems();

  /**
   * Returns a writable reference to the EquationSystems object helf within this MooseSystem
   */
  EquationSystems * getEquationSystems();

  /**
   * Returns a reference to the main nonlinear system in this instance of MooseSystem
   */
  TransientNonlinearImplicitSystem * getNonlinearSystem();
  
  /**
   * Returns a reference to the auxillary system in this instance of MooseSystem
   */
  TransientExplicitSystem * getAuxSystem();
  
  /**
   * Initialize all of the FE datastructures
   */
  void initDataStructures();

  /**
   * Check to see if MooseSystem is in a workable state before accessing data
   */
  void checkValid();

  /**
   * Get the Exodus Reader for this system.
   */
  ExodusII_IO * getExodusReader();

  unsigned int addVariable(const std::string &var, const FEType  &type, const std::set< subdomain_id_type  > *const active_subdomains=NULL);
  unsigned int addVariable(const std::string &var, const Order order=FIRST, const FEFamily=LAGRANGE, const std::set< subdomain_id_type > *const active_subdomains=NULL);

  void addKernel(std::string kernel_name,
                 std::string name,
                 InputParameters parameters);


  void addKernel(std::string kernel_name,
                 std::string name,
                 InputParameters parameters,
                 unsigned int block_id);


  void addBC(std::string bc_name,
             std::string name,
             InputParameters parameters);

  void addAuxKernel(std::string aux_name,
                    std::string name,
                    InputParameters parameters);

  // FIXME: was AuxKernel::addBC
  void addAuxBC(std::string aux_name,
                std::string name,
                InputParameters parameters);

  void addMaterial(std::string mat_name,
                   std::string name,
                   InputParameters parameters);

  void addStabilizer(std::string stabilizer_name,
                     std::string name,
                     InputParameters parameters);

  // FIXME: var_name should go to 'parameters'?
  void addInitialCondition(std::string ic_name,
                           std::string name,
                           InputParameters parameters,
                           std::string var_name);

  /**
   * Computes a block diagonal jacobian for the full system.
   */
  void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian);

  /**
   * Computes one block of the jacobian.
   *
   * @param ivar The block row to compute.
   * @param jvar The block column to compute.
   */
  void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);

  void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual);

  Number initial_value (const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& var_name);

  Gradient initial_gradient (const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& var_name);

  void initial_condition(EquationSystems& es, const std::string& system_name);

  void reinitKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke = NULL);

  void reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id);
  void reinitBCs(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual);

  void reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node);
  void reinitAuxKernels(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem);

  /**
   * Re-Initializes temporal discretization/transient control data.
   */
  void reinitDT();

  /**
   * Copy the old solutions backwards
   */
  void copy_old_solutions();

  /**
   * Re-Initializes Eigenvalue computation
   */
  void reinitEigen();

  void checkSystemsIntegrity();

  /**
   * Allows specification of per variable scaling factors.
   * The size of the vector MUST be the same as the number of Nonlinear Variables.
   * Should be called after Kernel::init() (because that sets the default scaling).
   * Can be called multiple times to change the scaling.
   * The initial scaling is just 1 for each variable.
   */
  void setVarScaling(std::vector<Real> scaling);

  /**
   * Get access to the active_local_element_range
   * Automatically builds it if it hasn't been initialized.
   */
  ConstElemRange * getActiveLocalElementRange();

  /**
   * Should be called after the mesh has been modified in any way.
   */
  void meshChanged();

  virtual void solve();

  /**
   * Get the EquationSystems params
   */
  Parameters &parameters();

  /**
   * Output the given system to output files.
   */
  void output_system(unsigned int t_step, Real time);


  void setPreconditioner(Preconditioner<Real> *pc) { _preconditioner = pc; }

  bool & dontReinitFE();
  
protected:
  void sizeEverything();
  void initKernels();
  void initBCs();
  void initAuxKernels();

private:
  std::vector<ElementData *> _element_data;
  std::vector<FaceData *> _face_data;
  std::vector<AuxData *> _aux_data;

  EquationSystems * _es;
  TransientNonlinearImplicitSystem * _system;
  TransientExplicitSystem * _aux_system;
  Mesh * _mesh;
  bool _delete_mesh;                            // true if we own the mesh and we are responsible for its destruction
  unsigned int _dim;

  /**
   * Whether or not the mesh has changed recently.  Useful for doing separate output.
   */
  bool _mesh_changed;

  KernelHolder _kernels;
  BCHolder _bcs;
  AuxHolder _auxs;
  MaterialHolder _materials;
  StabilizerHolder _stabilizers;
  ICHolder _ics;

  /**
   * Whether or not we need to recompute the shape functions for each element.  Should only be true if EVERY element is exactly
   * the same shape.
   */
  bool _no_fe_reinit;

  /**
   * Preconditioner
   */
  Preconditioner<Real> * _preconditioner;

  /**
   * Pointer to the material that is valid for the current block.
   */
  std::vector<Material *> _material;

  /**
   * The ExodusIO Reader to support reading of solutions at element qps
   */
  ExodusII_IO * _exreader;

  bool _is_valid;

  /**
   * Interior finite element.
   */
  std::vector<std::map<FEType, FEBase*> > _fe;

  /**
   * Maximum quadrature order required by all variables.
   */
  Order _max_quadrature_order;

  /**
   * Interior quadrature rule.
   */
  std::vector<QGauss *> _qrule;

  /**
   * Current element
   */
  std::vector<const Elem *> _current_elem;

  /**
   * Interior Jacobian pre-multiplied by the weight.
   */
  std::vector<std::map<FEType, const std::vector<Real> *> > _JxW;

  /**
   * Interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _phi;

  /**
   * Interior test function.
   *
   * Note that there is a different test function for each variable... allowing for modified
   * basis for things like SUPG and GLS.
   */
  std::vector<std::map<unsigned int, std::vector<std::vector<Real> > > > _test;

  /**
   * Gradient of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _dphi;

  /**
   * Second derivative of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _d2phi;

  /**
   * XYZ coordinates of quadrature points
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _q_point;

  /**
   * Variable numbers of the variables.
   */
  std::vector<unsigned int> _var_nums;

  /**
   * Variable numbers of the auxiliary variables.
   */
  std::vector<unsigned int> _aux_var_nums;

public:
  /**
   * Dof Maps for all the variables.
   */
  std::vector<std::vector<std::vector<unsigned int> > > _var_dof_indices;

protected:
  /**
   * Dof Maps for all the auxiliary variables.
   */
  std::vector<std::vector<std::vector<unsigned int> > > _aux_var_dof_indices;

  /**
   * Residual vectors for all variables.
   */
  std::vector<std::vector<DenseSubVector<Number> * > > _var_Res;

public:
  /**
   * Jacobian matrices for all variables.
   */
  std::vector<std::vector<DenseMatrix<Number> * > > _var_Kes;

protected:
  /**
   * Value of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<Real> > > _var_vals;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _var_grads;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealTensor> > > _var_seconds;

  /**
   * Value of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<Real> > > _var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  std::vector<std::vector<std::vector<Real> > > _var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _var_grads_older;

  /**
   * Value of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<Real> > > _aux_var_vals;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _aux_var_grads;

  /**
   * Value of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<Real> > > _aux_var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  std::vector<std::vector<std::vector<Real> > > _aux_var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _aux_var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _aux_var_grads_older;

public:
  /**
   * Current time.
   */
  Real _t;

  /**
   * Current dt.
   */
  Real _dt;

protected:

  /**
   * Old dt.
   */
  Real _dt_old;

  /**
   * Whether or not the current simulation is transient.
   */
  bool _is_transient;

  /**
   * Whether or not the current simulation is Eigenvalue.
   */
  bool _is_eigenvalue;

  /**
   * Current time step.
   */
  int _t_step;

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  std::vector<Real> _bdf2_wei;

  /**
   * Time discretization scheme: 0 - Implicit Euler, 1 - 2nd-order Backward Difference
   */
  short _t_scheme;

  /**
   * The total number of Runge-Kutta stages
   */
  short _n_of_rk_stages;


  DofMap * _dof_map;
  DofMap * _aux_dof_map;
  std::vector<std::vector<unsigned int> > _dof_indices;
  std::vector<std::vector<unsigned int> > _aux_dof_indices;

  /**
   * Scaling factors for each variable.
   */
  std::vector<Real> _scaling_factor;


  /// BCs
  /**
   * Current node for nodal BC's
   */
  std::vector<const Node *> _current_node;

  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  std::vector<NumericVector<Number> *> _current_residual;

  /**
   * Current side.
   */
  std::vector<unsigned int> _current_side;

  /**
   * Boundary finite element.
   */
  std::vector<std::map<FEType, FEBase *> > _fe_face;

  /**
   * Boundary quadrature rule.
   */
  std::vector<QGauss *> _qface;

  /**
   * XYZ coordinates of quadrature points
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _q_point_face;

  /**
   * Side Jacobian pre-multiplied by the weight.
   */
  std::vector<std::map<FEType, const std::vector<Real> *> > _JxW_face;

  /**
   * Side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _phi_face;

  /**
   * Gradient of side shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _dphi_face;

  /**
   * Second derivative of interior shape function.
   */
  std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _d2phi_face;

  /**
   * Normal vectors at the quadrature points.
   */
  std::vector<std::map<FEType, const std::vector<Point> *> > _normals_face;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the quadrature points on that boundary
   */
  std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the nodes on that boundary
   */
  std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums_nodal;

  /**
   * Holds the current dof numbers for each variable for nodal bcs
   */
  std::vector<std::vector<unsigned int> > _nodal_bc_var_dofs;

  /**
   * ***************
   * Values of stuff
   * ***************
   */

  /**
   * Value of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<Real> > > _var_vals_face;

  /**
   * Gradient of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealGradient> > > _var_grads_face;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  std::vector<std::vector<std::vector<RealTensor> > > _var_seconds_face;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<std::vector<Real> > > _var_vals_face_nodal;

  /**
   * Convenience zeros.
   */
  std::vector<Real> _real_zero;
  std::vector<std::vector<Real> > _zero;
  std::vector<std::vector<RealGradient> > _grad_zero;
  std::vector<std::vector<RealTensor> > _second_zero;

  /**
   * A range for use with TBB.  We do this so that it doesn't have
   * to get rebuilt all the time (which takes time).
   */
  ConstElemRange * active_local_elem_range;

  // Aux Kernels
  const NumericVector<Number> * _nonlinear_old_soln;
  const NumericVector<Number> * _nonlinear_older_soln;

  NumericVector<Number> * _aux_soln;
  const NumericVector<Number> * _aux_old_soln;
  const NumericVector<Number> * _aux_older_soln;

  /**
   * Holds the current dof numbers for each variable
   */
  std::vector<std::vector<unsigned int> > _aux_var_dofs;

  /**
   * Holds the variable numbers of the nodal aux vars.
   */
  std::vector<unsigned int> _nodal_var_nums;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<Real > > _var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<Real > > _var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<std::vector<Real > > _var_vals_older_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<Real > > _aux_var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  std::vector<std::vector<Real > > _aux_var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  std::vector<std::vector<Real > > _aux_var_vals_older_nodal;


  /*****************
   * Elemental Stuff
   *****************/

  /**
   * Holds the variable numbers of the elemental aux vars.
   */
  std::vector<unsigned int> _element_var_nums;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<Real > > _var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<Real > > _var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  std::vector<std::vector<Real > > _var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<RealGradient > > _var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<RealGradient > > _var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  std::vector<std::vector<RealGradient > > _var_grads_older_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<Real > > _aux_var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  std::vector<std::vector<Real > > _aux_var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  std::vector<std::vector<Real > > _aux_var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<RealGradient > > _aux_var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  std::vector<std::vector<RealGradient > > _aux_var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  std::vector<std::vector<RealGradient > > _aux_var_grads_older_element;


  Real integrateValueAux(const std::vector<Real> & vals, const std::vector<Real> & JxW, const std::vector<Point> & q_point);
  RealGradient integrateGradientAux(const std::vector<RealGradient> & grads, const std::vector<Real> & JxW, const std::vector<Point> & q_point);


  void update_aux_vars(const NumericVector<Number>& soln);

  /**
   * Computes the value of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in transient problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                            std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                            std::vector<RealTensor> &second_u,
                            const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi);

  /**
   * Computes the value of all the soln and gradient at the current quadrature point in transient problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                            std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                            const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in steady state problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(std::vector<Real> & u,
                            std::vector<RealGradient> &grad_u,
                            std::vector<RealTensor> &second_u,
                            const NumericVector<Number> & soln,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi, const std::vector<std::vector<RealTensor> > & d2phi);

  /**
   * Computes the value of all the soln and gradient at the current quadrature point in steady state problems.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSolutionAll(std::vector<Real> & u,
                            std::vector<RealGradient> &grad_u,
                            const NumericVector<Number> & soln,
                            const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                            const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the gradient of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the second derivative of soln at the current quadrature point.
   *
   * @param soln The solution vector to pull the coefficients from.
   */
  void computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi);

  friend class ComputeInternalJacobians;
  friend class ComputeInternalJacobianBlocks;
  friend class ComputeInternalResiduals;

  friend class Kernel;
  friend class AuxKernel;
  friend class BoundaryCondition;
  friend class Material;
  friend class Stabilizer;
  friend class Executioner;
  friend class TransientExecutioner;
};

#endif //MOOSESYSTEM_H
