// libMesh includes
#include "equation_systems.h"
#include "mesh_base.h"
#include "nonlinear_implicit_system.h"
#include "fe_base.h"
#include "quadrature_gauss.h"
#include "transient_system.h"
#include "parameters.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"

//Forward Declarations
class Elem;
class Material;

#ifndef KERNEL_H
#define KERNEL_H

template<class KernelType>
Parameters valid_params()
{
  Parameters params;
  return params;
}

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
         Parameters parameters,
         std::string var_name,
         bool integrated,
         std::vector<std::string> coupled_to = std::vector<std::string>(0),
         std::vector<std::string> coupled_as = std::vector<std::string>(0));

  virtual ~Kernel()
  {};

  /**
   * Initializes common data structures.
   */
  static void init(EquationSystems * es);

  /**
   * Re-Initializes common data structures for a specific element.
   */
  static void reinit(const NumericVector<Number>& soln, const Elem * elem, DenseVector<Number> * Re, DenseMatrix<Number> * Ke = NULL);

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
   * Computes the volume integral for the current element.
   */
  virtual Real computeIntegral();

  static DofMap * _dof_map;
  static std::vector<unsigned int> _dof_indices;

  /**
   * Retrieve name of the Kernel
   */
  std::string name();

protected:
  /**
   * This Kernel's name.
   */
  std::string _name;
  
  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  Parameters _parameters;

  /** 
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual()=0;

  /** 
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian()
  {
    return 0;
  }

  /** 
   * This is the virtual that derived classes should override for computing the volume integral of kernel.
   */
  virtual Real computeQpIntegral()
  {
    return 0;
  }

  /**
   * Name of the variable being solved for.
   */
  std::string _var_name;

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
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  bool isCoupled(std::string name);
  
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
   * ***********************
   * All of the static stuff
   * ***********************
   */
  
  static EquationSystems * _es;
  static TransientNonlinearImplicitSystem * _system;
  static MeshBase * _mesh;
  static unsigned int _dim;

  /**
   * FE Type to be used.
   */
  static FEType _fe_type;

  /**
   * Interior finite element.
   */
  static AutoPtr<FEBase> _fe;

  /**
   * Interior quadrature rule.
   */
  static QGauss * _qrule;  

  /**
   * Current element
   */
  static const Elem * _current_elem;

  /**
   * Interior Jacobian pre-multiplied by the weight.
   */
  static const std::vector<Real> * _static_JxW;

  /**
   * Interior shape function.
   */
  static const std::vector<std::vector<Real> > * _static_phi;

  /**
   * Gradient of interior shape function.
   */
  static const std::vector<std::vector<RealGradient> > * _static_dphi;

  /**
   * XYZ coordinates of quadrature points
   */
  static const std::vector<Point> * _static_q_point;
  
  /**
   * Variable numbers of the variables.
   */
  static std::vector<unsigned int> _var_nums;

  /**
   * Dof Maps for all the variables.
   */
  static std::map<unsigned int, std::vector<unsigned int> > _var_dof_indices;

  /**
   * Residual vectors for all variables.
   */
  static std::map<unsigned int, DenseSubVector<Number> * > _var_Res;

  /**
   * Jacobian matrices for all variables.
   */
  static std::map<unsigned int, DenseSubMatrix<Number> * > _var_Kes;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::map<unsigned int, std::vector<Real> > _var_vals;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::map<unsigned int, std::vector<RealGradient> > _var_grads;

  /**
   * Value of the variables at the quadrature points.
   */
  static std::map<unsigned int, std::vector<Real> > _var_vals_old;

  /**
   * Value of the variables at the quadrature points at t-2.
   */
  static std::map<unsigned int, std::vector<Real> > _var_vals_older;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::map<unsigned int, std::vector<RealGradient> > _var_grads_old;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::map<unsigned int, std::vector<RealGradient> > _var_grads_older;
  
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
  static Material * _material;

  /**
   * Computes the value of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpSolution(Real & u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<Real> > & phi);

  /**
   * Computes the value of the gradient of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  static void computeQpGradSolution(RealGradient & grad_u, const NumericVector<Number> & soln, const std::vector<unsigned int> & dof_indices, const unsigned int qp, const std::vector<std::vector<RealGradient> > & dphi);
};

#endif //KERNEL_H
