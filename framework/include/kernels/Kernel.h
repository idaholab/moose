#ifndef KERNEL_H
#define KERNEL_H

// local includes
#include "Moose.h"
#include "ValidParams.h"

// libMesh includes
#include "equation_systems.h"
#include "mesh_base.h"
#include "nonlinear_implicit_system.h"
#include "fe_base.h"
#include "quadrature_gauss.h"
#include "transient_system.h"
#include "InputParameters.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"
#include "tensor_value.h"

//Forward Declarations
class Elem;
class Material;

/*
template<class KernelType>
InputParameters validParams()
{
  InputParameters params;
  params.set<bool>("junk") = true;
  return params;
}
*/

/** 
 * The Kernel class is responsible for calculating the residuals for various
 * physics.
 * 
 */
class Kernel
{
public:

  /** 
   * Factory constrcutor initializes all internal references needed for residual computation.
   * 
   *
   * @param name The name of this kernel.
   * @param system The system this variable is in
   * @param var_name The variable this Kernel is going to compute a residual for.
   * @param integrated Whether or not the residual is integraded (used by BCs).
   * @param coupled_to The names of the variables this Kernel is coupled_to
   * @param coupled_as The names of the variables the Kernel is going to ask for.
   */
  Kernel(std::string name,
         InputParameters parameters,
         std::string var_name,
         bool integrated,
         std::vector<std::string> coupled_to = std::vector<std::string>(0),
         std::vector<std::string> coupled_as = std::vector<std::string>(0));

  virtual ~Kernel()
  {};

  /**
   * Size everything.
   */
  static void sizeEverything();

  /**
   * Initializes common data structures.
   */
  static void init(EquationSystems * es);

  /**
   * Re-Initializes common data structures for a specific element.
   */
  static void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke = NULL);

  /**
   * Re-Initializes temporal discretization/transient control data.
   */
  static void reinitDT();

  /**
   * Re-Initializes Eigenvalue computation
   */
  static void reinitEigen();

  /** 
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /** 
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar);

  /** 
   * Computes the volume integral for the current element.
   */
  virtual Real computeIntegral();

  static DofMap * _dof_map;
  static DofMap * _aux_dof_map;
  static std::vector<std::vector<unsigned int> > _dof_indices;
  static std::vector<std::vector<unsigned int> > _aux_dof_indices;

  /**
   * Retrieve name of the Kernel
   */
  std::string name() const;

  /**
   * Retrieve the name of the variable that this Kernel operates on
   */
  std::string varName() const;

  /**
   * Retrieve the names of the variables this Kernel is coupled to
   */
  const std::vector<std::string> & coupledTo() const;

  /**
   * This virtual gets called every time the subdomain changes.  This is useful for doing pre-calcualtions
   * that should only be done once per subdomain.  In particular this is where references to material
   * property vectors should be initialized.
   */
  virtual void subdomainSetup();

  /**
   * The variable number that this kernel operates on.
   */
  unsigned int variable();

  /**
   * Return the thread id this kernel is associated with.
   */
  THREAD_ID tid();

  /**
   * Computes the modified variable number for an auxiliary variable.
   * This is the variable number that Kernels know this variable to operate under.
   *
   * This is necessary because Kernels need unique variable numbers for computing
   * off-diagonal jacobian components.
   */
  static bool modifiedAuxVarNum(unsigned int var_num);

  /**
   * Allows specification of per variable scaling factors.
   * The size of the vector MUST be the same as the number of Nonlinear Variables.
   * Should be called after Kernel::init() (because that sets the default scaling).
   * Can be called multiple times to change the scaling.
   * The initial scaling is just 1 for each variable.
   */
  static void setVarScaling(std::vector<Real> scaling);
  
protected:
  /**
   * This Kernel's name.
   */
  std::string _name;

  /**
   * The thread id this kernel is associated with.
   */
  THREAD_ID _tid;
  
  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

  /** 
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual()=0;

  /** 
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian();
  
  /** 
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
  /** 
   * This is the virtual that derived classes should override for computing the volume integral of kernel.
   */
  virtual Real computeQpIntegral();
  
  /**
   * Name of the variable being solved for.
   */
  std::string _var_name;

  /**
   * Whether or not this kernel is operating on an auxiliary variable.
   */
  bool _is_aux;

  /**
   * System variable number for this variable.
   */
  unsigned int _var_num;

  /**
   * If false the result of computeQpResidual() will overwrite the current Re entry instead of summing.
   * Right now it's only really used for computeSideResidual so Derichlet BC's can be computed exactly.
   */
  bool _integrated;

  /**
   * Holds the current solution at the current quadrature point.
   */
  std::vector<Real> & _u;

  /**
   * The value of _u at a nodal position.  Used by non-integrated boundaries.
   */
  Real _u_node;

  /**
   * Holds the current solution gradient at the current quadrature point.
   */
  std::vector<RealGradient> & _grad_u;

  /**
   * Holds the current solution second derivative at the current quadrature point.
   */
  std::vector<RealTensor> & _second_u;

  /**
   * Holds the previous solution at the current quadrature point.
   */
  std::vector<Real> & _u_old;

  /**
   * Holds the t-2 solution at the current quadrature point.
   */
  std::vector<Real> & _u_older;

  /**
   * Holds the previous solution gradient at the current quadrature point.
   */
  std::vector<RealGradient> & _grad_u_old;

  /**
   * Holds the t-2 solution gradient at the current quadrature point.
   */
  std::vector<RealGradient> & _grad_u_older;

  /**
   * The Finite Element type corresponding to the variable this
   * Kernel operates on.
   */
  FEType _fe_type;

  /**
   * Current element
   */
  const Elem * & _current_elem;

  /**
   * Current material
   */
  Material * & _material;
  
  /**
   * Interior Jacobian pre-multiplied by the weight.
   */
  const std::vector<Real> & _JxW;

  /**
   * Interior shape function.
   */
  const std::vector<std::vector<Real> > & _phi;

  /**
   * Gradient of interior shape function.
   */
  const std::vector<std::vector<RealGradient> > & _dphi;

  /**
   * Second derivative of interior shape function.
   */
  const std::vector<std::vector<RealTensor> > & _d2phi;

  /**
   * Current quadrature rule.
   */
  QGauss * & _qrule;  

  /**
   * XYZ coordinates of quadrature points
   */
  const std::vector<Point>& _q_point;

  /**
   * Current shape function.
   */
  unsigned int _i;

  /**
   * Current shape function while computing jacobians.
   * This should be used for the variable's shape functions, while _i
   * is used for the test function.
   */
  unsigned int _j;

  /**
   * Current _qrule quadrature point.
   */
  unsigned int _qp;

  /**
   * Variable numbers of the coupled variables.
   */
  std::vector<unsigned int> _coupled_var_nums;

  /**
   * Variable numbers of the coupled auxiliary variables.
   */
  std::vector<unsigned int> _aux_coupled_var_nums;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_to;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_as;

  /**
   * Map from _as_ to the actual variable number.
   */
  std::map<std::string, unsigned int> _coupled_as_to_var_num;

  /**
   * Map from _as_ to the actual variable number for auxiliary variables.
   */
  std::map<std::string, unsigned int> _aux_coupled_as_to_var_num;

  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  bool isCoupled(std::string name);

  /**
   * Returns the variable number of the coupled variable.
   */
  unsigned int coupled(std::string name);
  
  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<Real> & coupledVal(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<RealGradient> & coupledGrad(std::string name);
  
  /**
   * Returns a reference (that can be stored) to a coupled variable's second derivative.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<RealTensor> & coupledSecond(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at old time step.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<Real> & coupledValOld(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at older time step.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<Real> & coupledValOlder(std::string name);

  /**
   * Returns a referene (that can be sotred) to a coupled gradient of a variable's value at an old time step.
   *
   * @param name The name the kernel wants to refer to the variable as
   */
  
  std::vector<RealGradient> & coupledGradValOld(std::string name);
  
  /**
   * Just here for convenience.  Used in constructors... usually to deal with multiple dimensional stuff.
   */
  Real & _real_zero;
  std::vector<Real> & _zero;
  std::vector<RealGradient> & _grad_zero;
  std::vector<RealTensor> & _second_zero;

  /**
   * Whether or not the variable this Kernel operates on supports second derivatives.
   */
  bool _has_second_derivatives;

  /**
   * ***********************
   * All of the static stuff
   * ***********************
   */
  
  static EquationSystems * _es;
  static TransientNonlinearImplicitSystem * _system;
  static TransientExplicitSystem * _aux_system;
  static MeshBase * _mesh;
  static unsigned int _dim;

  /**
   * Interior finite element.
   */
  static std::vector<std::map<FEType, FEBase*> > _fe;

  /**
   * Maximum quadrature order required by all variables.
   */
  static Order _max_quadrature_order;

  /**
   * Interior quadrature rule.
   */
  static std::vector<QGauss *> _static_qrule;  

  /**
   * Current element
   */
  static std::vector<const Elem *> _static_current_elem;

  /**
   * Interior Jacobian pre-multiplied by the weight.
   */
  static std::vector<std::map<FEType, const std::vector<Real> *> > _static_JxW;

  /**
   * Interior shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _static_phi;

  /**
   * Gradient of interior shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _static_dphi;

  /**
   * Second derivative of interior shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _static_d2phi;

  /**
   * XYZ coordinates of quadrature points
   */
  static std::vector<std::map<FEType, const std::vector<Point> *> > _static_q_point;
  
  /**
   * Variable numbers of the variables.
   */
  static std::vector<unsigned int> _var_nums;

  /**
   * Variable numbers of the auxiliary variables.
   */
  static std::vector<unsigned int> _aux_var_nums;

  /**
   * Dof Maps for all the variables.
   */
  static std::vector<std::map<unsigned int, std::vector<unsigned int> > > _var_dof_indices;

  /**
   * Dof Maps for all the auxiliary variables.
   */
  static std::vector<std::map<unsigned int, std::vector<unsigned int> > > _aux_var_dof_indices;

  /**
   * Residual vectors for all variables.
   */
  static std::vector<std::map<unsigned int, DenseSubVector<Number> * > > _var_Res;

  /**
   * Jacobian matrices for all variables.
   */
  static std::vector<std::map<unsigned int, DenseSubMatrix<Number> * > > _var_Kes;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _var_vals;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _var_grads;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealTensor> > > _var_seconds;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _var_grads_older;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _aux_var_vals;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _aux_var_grads;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _aux_var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  static std::vector<std::map<unsigned int, std::vector<Real> > > _aux_var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _aux_var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::map<unsigned int, std::vector<RealGradient> > > _aux_var_grads_older;
  
  /**
   * Current time.
   */
  static Real _t;

  /**
   * Current dt.
   */
  static Real _dt;

  /**
   * Old dt.
   */
  static Real _dt_old;
  
  /**
   * Whether or not the current simulation is transient.
   */
  static bool _is_transient;

  /**
   * Whether or not the current simulation is Eigenvalue.
   */
  static bool _is_eigenvalue;

  /**
   * Current time step.
   */
  static int _t_step;

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  static Real _bdf2_wei[3];

  /**
   * Time discretization scheme: 0 - Implicit Euler, 1 - 2nd-order Backward Difference
   */
  static short _t_scheme;

  /**
   * The total number of Runge-Kutta stages
   */
  static short _n_of_rk_stages;
  
  /**
   * Pointer to the material that is valid for the current block.
   */
  static std::vector<Material *> _static_material;

  /**
   * Computes the value of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in transient problems.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
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
  static void computeQpSolutionAll(std::vector<Real> & u, std::vector<Real> & u_old, std::vector<Real> & u_older,
                                   std::vector<RealGradient> &grad_u,  std::vector<RealGradient> &grad_u_old, std::vector<RealGradient> &grad_u_older,
                                   const NumericVector<Number> & soln, const NumericVector<Number> & soln_old,  const NumericVector<Number> & soln_older,
                                   const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                   const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of all the soln, gradient and second derivative at the current quadrature point in steady state problems.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpSolutionAll(std::vector<Real> & u,
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
  static void computeQpSolutionAll(std::vector<Real> & u,
                                   std::vector<RealGradient> &grad_u,
                                   const NumericVector<Number> & soln,
                                   const std::vector<unsigned int> & dof_indices, const unsigned int n_qp,
                                   const std::vector<std::vector<Real> > & phi, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the gradient of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi);

  /**
   * Computes the value of the second derivative of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpSecondSolution(RealTensor & second_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealTensor> > & d2phi);

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
  bool isAux(std::string name);

  /**
   * Static convenience zeros.
   */
  static std::vector<Real> _static_real_zero;
  static std::vector<std::vector<Real> > _static_zero;
  static std::vector<std::vector<RealGradient> > _static_grad_zero;
  static std::vector<std::vector<RealTensor> > _static_second_zero;

  /**
   * Scaling factors for each variable.
   */
  static std::vector<Real> _scaling_factor;
};

#endif //KERNEL_H
