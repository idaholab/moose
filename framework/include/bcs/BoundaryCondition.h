#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

#include "Kernel.h"

//forward declarations
class BoundaryCondition;

template<>
InputParameters validParams<BoundaryCondition>();

/** 
 * Extrapolation of a Kernel for BC usage.  Children of this class should override
 * computeQpResidual() for use by computeSideResidual.
 */
class BoundaryCondition : public Kernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  BoundaryCondition(std::string name,
                    MooseSystem & moose_system,
                    InputParameters parameters);

  virtual ~BoundaryCondition(){}

  /**
   * Size everything.
   */
  static void sizeEverything();

  /**
   * Initializes common data structures.
   */
  static void init();

  /**
   * Reinitializes common datastructures
   * Must be called after a Kernel::reinit() for the same element
   */
  static void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const unsigned int side, const unsigned int boundary_id);

  /**
   * Reinitializes common datastructures for nodal bcs.
   */
  static void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node, const unsigned int boundary_id, NumericVector<Number>& residual);

  /** 
   * Boundary ID the BC is active on.
   * 
   * @return The boundary ID.
   */
  unsigned int boundaryID();

  /** 
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /** 
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

  /**
   * Computes d-ivar-residual / d-jvar... storing the result in Ke.
   */
  void computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar);

  /**
   * For use with non-integrated / nodal (Dirichlet) BC's
   */
  void computeAndStoreResidual();

  /** 
   * Computes the integral over the current side.
   */
  Real computeIntegral();

  /**
   * Whether or not the BC is integrated over the boundary.
   */
  bool isIntegrated();

  /**
   * Sets the "_integrated" parameter and returns a reference for use in the parent constructor
   */
  InputParameters & setIntegratedParam(InputParameters & params, bool integrated);

protected:

  /**
   * Reference to the MooseSystem that this BoundaryCondition is assocaited to
   */
  MooseSystem & _moose_system;
  
  /**
   * Boundary ID this BC is active on.
   */
  unsigned int _boundary_id;

  /**
   * The current side as an element
   * Only valid if there is a Dirichlet BC
   * on this side.
   */
  Elem * _side_elem;

  /**
   * Side Jacobian pre-multiplied by the weight.
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
   * Second derivative of side shape function.
   */
  const std::vector<std::vector<RealTensor> > & _d2phi_face;

  /**
   * Normal vectors at the quadrature points.
   */
  const std::vector<Point>& _normals_face;

  /**
   * Boundary quadrature rule.
   */
  QGauss * & _qface;
  
  /**
   * XYZ coordinates of quadrature points
   */
  const std::vector<Point>& _q_point_face;

  /**
   * Current side.
   */
  unsigned int & _current_side;

  /**
   * Current node for nodal BC's
   */
  const Node * & _current_node;
  
  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  NumericVector<Number> * & _current_residual;

  /**
   * ***********************
   * All of the static stuff
   * ***********************
   */

  /**
   * Current node for nodal BC's
   */
  static std::vector<const Node *> _static_current_node;
  
  /**
   * Current residual vector.  Only valid for nodal BC's.
   */
  static std::vector<NumericVector<Number> *> _static_current_residual;
    
  /**
   * Current side.
   */
  static std::vector<unsigned int> _static_current_side;
  
  /**
   * Boundary finite element. 
   */
  static std::vector<std::map<FEType, FEBase *> > _fe_face;

  /**
   * Boundary quadrature rule.
   */
  static std::vector<QGauss *> _static_qface;

  /**
   * XYZ coordinates of quadrature points
   */
  static std::vector<std::map<FEType, const std::vector<Point> *> > _static_q_point_face;

  /**
   * Side Jacobian pre-multiplied by the weight.
   */
  static std::vector<std::map<FEType, const std::vector<Real> *> > _static_JxW_face;

  /**
   * Side shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<Real> > *> > _static_phi_face;

  /**
   * Gradient of side shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<RealGradient> > *> > _static_dphi_face;

  /**
   * Second derivative of interior shape function.
   */
  static std::vector<std::map<FEType, const std::vector<std::vector<RealTensor> > *> > _static_d2phi_face;

  /**
   * Normal vectors at the quadrature points.
   */
  static std::vector<std::map<FEType, const std::vector<Point> *> > _static_normals_face;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the quadrature points on that boundary
   */
  static std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums;

  /**
   * Map to vector of variable numbers that need to be evaluated
   * at the nodes on that boundary
   */
  static std::map<unsigned int, std::vector<unsigned int> > _boundary_to_var_nums_nodal;
  
  /**
   * Holds the current dof numbers for each variable for nodal bcs
   */
  static std::vector<std::vector<unsigned int> > _nodal_bc_var_dofs;

  /**
   * ***************
   * Values of stuff
   * ***************
   */
  
  /**
   * Value of the variables at the quadrature points.
   */
  static std::vector<std::vector<std::vector<Real> > > _var_vals_face;

  /**
   * Gradient of the variables at the quadrature points.
   */
  static std::vector<std::vector<std::vector<RealGradient> > > _var_grads_face;

  /**
   * Second derivatives of the variables at the quadrature points.
   */
  static std::vector<std::vector<std::vector<RealTensor> > > _var_seconds_face;

  /**
   * Value of the variables at the nodes.
   */
  static std::vector<std::vector<std::vector<Real> > > _var_vals_face_nodal;

  /**
   * Holds the current solution at the current quadrature point on the face.
   */
  std::vector<Real> & _u_face;

  /**
   * Holds the current solution gradient at the current quadrature point on the face.
   */
  std::vector<RealGradient> & _grad_u_face;

  /**
   * Holds the current solution second derivative at the current quadrature point on the face
   */
  std::vector<RealTensor> & _second_u_face;

  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<Real> & coupledValFace(std::string name);  

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<RealGradient> & coupledGradFace(std::string name);

private:
  
  /**
   * Constructor helper, adds all var_nums associated with this
   * BC to the appropriate list.
   */
  void addVarNums(std::vector<unsigned int> & var_nums);

  /**
   * Stuff we don't want BC classes to get access to from Kernel
   */
  Kernel::_u;
  Kernel::_u_old;
  Kernel::_u_older;
  Kernel::_grad_u;
  Kernel::_grad_u_old;
  Kernel::_grad_u_older;
  Kernel::_second_u;
  Kernel::_JxW;
  Kernel::_phi;
  Kernel::_dphi;
  Kernel::_d2phi;
  Kernel::_q_point;
  Kernel::coupledVal;
  Kernel::coupledGrad;
};

#endif //BOUNDARYCONDITION_H
