// libMesh includes
#include "equation_systems.h"
#include "mesh_base.h"
#include "nonlinear_implicit_system.h"
#include "fe_base.h"
#include "quadrature_gauss.h"
#include "transient_system.h"

//Forward Declarations
class Elem; 

#ifndef KERNEL_H
#define KERNEL_H

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
   * @param system The system this variable is in
   * @param var_name The variable this Kernel is going to compute a residual for.
   * @param integrated Whether or not the residual is integraded (used by BCs).
   */
  Kernel(Parameters parameters, EquationSystems * es, std::string var_name, bool integrated=true, std::vector<std::string> coupled_to=0);

  /** 
   * Standalone constructor initializes all internal references needed for residual computation.
   * 
   * @param system The system this variable is in
   * @param var_name The variable this Kernel is going to compute a residual for.
   * @param integrated Whether or not the residual is integraded (used by BCs).
   */
  Kernel(EquationSystems * es, std::string var_name, bool integrated=true, std::vector<std::string> coupled_to=0);

  virtual ~Kernel()
  {
    
  };

  /** 
   * Computes the residual for the current element.
   * 
   * @param Re Local residual vector.
   * @param elem Current element.
   */
  void computeElemResidual(const NumericVector<Number>& soln, DenseVector<Number> & Re, const Elem * elem);

  /** 
   * Computes the residual for the current side.
   * 
   * @param Re Local residual vector.
   * @param elem Current element.
   * @param side Current side.
   */
  void computeSideResidual(const NumericVector<Number>& soln, DenseVector<Number> & Re, const Elem * elem, unsigned int side);


protected:
  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  Parameters _parameters;

  /** 
   * This is the virtual that derived classes should override.
   */
  virtual Real computeQpResidual()=0;

  /** 
   * Piece of the residual that gets added for transient solves.
   */
  virtual Real computeQpTransientResidual(){}

  /**
   * If false the result of computeQpResidual() will overwrite the current Re entry instead of summing.
   * Right now it's only really used for computeSideResidual so Derichlet BC's can be computed exactly.
   */
  bool _integrated;

  /**
   * Holds the current solution at the current quadrature point.
   */
  Real _u;

  /**
   * Holds the current solution gradient at the current quadrature point.
   */
  RealGradient _grad_u;

  /**
   * Holds the previous solution at the current quadrature point.
   */
  Real _u_old;

  /**
   * Holds the previous solution gradient at the current quadrature point.
   */
  RealGradient _grad_u_old;

  EquationSystems & _es;
  std::string _var_name;

  MeshBase & _mesh;
  unsigned int _dim;

  TransientNonlinearImplicitSystem & _system;

  unsigned int _var_num;
  unsigned int _var_num_dofs;

public:
  const DofMap & _dof_map;
  std::vector<unsigned int> _dof_indices;

protected:
  /**
   * This variable's dof_indices
   */
  std::vector<unsigned int> _var_dof_indices;

  /**
   * FE Type to be used.
   */
  FEType _fe_type;

  /**
   * Interior finite element.
   */
  AutoPtr<FEBase> _fe;

  /**
   * Interior quadrature rule.
   */
  QGauss _qrule;

  /**
   * Current _qrule quadrature point.
   */
  unsigned int _qp;

  /**
   * Boundary finite element. 
   */
  AutoPtr<FEBase> _fe_face;

  /**
   * Boundary quadrature rule.
   */
  QGauss _qface;

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
   * Interior Jacobian pre-multiplied by the weight.
   */
  const std::vector<Real> & _JxW_face;

  /**
   * Side shape function.
   */
  const std::vector<std::vector<Real> > & _phi_face;

  /**
   * Gradient of side shape function.
   */
  const std::vector<std::vector<RealGradient> > & _dphi_face;

  /**
   * Current shape function.
   */
  unsigned int _i;

  /**
   * Current time.
   */
  Real _t;

  /**
   * Current dt.
   */
  Real _dt;

  /**
   * Whether or not the current simulation is transient.
   */
  bool _is_transient;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_to;

  /**
   * Variable numbers of the coupled variables.
   */
  std::vector<unsigned int> _coupled_var_nums;

  /**
   * Dof Maps for coupled variables.
   */
  std::vector<std::vector<unsigned int> > _coupled_dof_indices;

  /**
   * Value of the coupled variables at the quadrature points.
   */
  std::map<std::string, Real> _coupled_vals;

  /**
   * Gradient of the coupled variables at the quadrature points.
   */
  std::map<std::string, RealGradient> _coupled_grads;

private:
  /**
   * Computes the value of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  Real computeQpSolution(const NumericVector<Number>& soln, const std::vector<unsigned int>& dof_indices);

  /**
   * Computes the value of the gradient of soln at the current quadrature point.
   * 
   * @param soln The solution vector to pull the coefficients from.
   */
  RealGradient computeQpGradSolution(const NumericVector<Number>& soln, const std::vector<unsigned int>& dof_indices);

};

#endif //KERNEL_H
