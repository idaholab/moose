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
class Kernel;
class MooseSystem;
class ElementData;

template<>
InputParameters validParams<Kernel>();

/** 
 * The Kernel class is responsible for calculating the residuals for various
 * physics.
 * 
 */
class Kernel
{
public:

  /** 
   * Factory constructor initializes all internal references needed for residual computation.
   * 
   *
   * @param name The name of this kernel.
   * @param moose_system The moose_system this kernel is associated with
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  Kernel(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~Kernel()
  {};

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
   * The time, after which this kernel will be active.
   */
  Real startTime();

  /**
   * The time, after which this kernel will be inactive.
   */
  Real stopTime();

  /**
   * Computes the modified variable number for an auxiliary variable.
   * This is the variable number that Kernels know this variable to operate under.
   *
   * This is necessary because Kernels need unique variable numbers for computing
   * off-diagonal jacobian components.
   */
  static unsigned int modifiedAuxVarNum(unsigned int var_num);

protected:
  /**
   * This Kernel's name.
   */
  std::string _name;

  /**
   * Reference to the MooseSystem that this Kernel is assocaited to
   */
  MooseSystem & _moose_system;

  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;
  
  /**
   * The thread id this kernel is associated with.
   */
  THREAD_ID _tid;
  
  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

  /**
   * The mesh.
   */
  Mesh & _mesh;

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
   * The current dimension of the mesh.
   */
  unsigned int & _dim;

  /**
   * Current time.
   */
  Real & _t;
  
  /**
   * Current dt.
   */
  Real & _dt;

  /**
   * Old dt.
   */
  Real & _dt_old;

  /**
   * Whether or not the current simulation is transient.
   */
  bool & _is_transient;

  /**
   * Whether or not the current simulation is Eigenvalue.
   */
  bool & _is_eigenvalue;

  /**
   * Current time step.
   */
  int & _t_step;

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  std::vector<Real> & _bdf2_wei;

  /**
   * Time discretization scheme: 0 - Implicit Euler, 1 - 2nd-order Backward Difference
   */
  short & _t_scheme;

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
   * Interior test function.
   *
   * These are non-const so they can be modified for stabilization.
   */
  std::vector<std::vector<Real> > & _test;

  /**
   * Gradient of interior shape function.
   */
  const std::vector<std::vector<RealGradient> > & _dphi;

  /**
   * Gradient of interior test function.
   */
  const std::vector<std::vector<RealGradient> > & _dtest;

  /**
   * Second derivative of interior shape function.
   */
  const std::vector<std::vector<RealTensor> > & _d2phi;

  /**
   * Second derivative of interior test function.
   */
  const std::vector<std::vector<RealTensor> > & _d2test;

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
   * Returns a reference (that can be stored) to a coupled gradient of a variable's value at an old time step.
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
   * The time, after which this kernel will be active.
   */
  Real _start_time;

  /**
   * The time, after which this kernel will be inactive.
   */
  Real _stop_time;

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
  bool isAux(std::string name);
};

#endif //KERNEL_H
