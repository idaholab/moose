#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

// local includes
#include "Moose.h"
#include "ValidParams.h"
#include "MooseArray.h"
#include "MaterialPropertyInterface.h"

// libMesh includes
#include "mesh_base.h"
#include "fe_base.h"
#include "quadrature_gauss.h"
#include "InputParameters.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"

//forward declarations
class BoundaryCondition;
class MooseSystem;
class ElementData;
class FaceData;
class AuxData;

template<>
InputParameters validParams<BoundaryCondition>();

/** 
 * Extrapolation of a Kernel for BC usage.  Children of this class should override
 * computeQpResidual() for use by computeSideResidual.
 */
class BoundaryCondition : protected MaterialPropertyInterface
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  BoundaryCondition(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~BoundaryCondition(){}

  /**
   * The variable number that this kernel operates on.
   */
  unsigned int variable();

  /**
   * Return the thread id this kernel is associated with.
   */
  THREAD_ID tid();

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
   * Convenience reference to the FaceData object inside of MooseSystem
   */
  FaceData & _face_data;

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
   * The Finite Element type corresponding to the variable this
   * Kernel operates on.
   */
  FEType _fe_type;

  /**
   * Current element
   */
  const Elem * & _current_elem;

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
   * Current material
   */
  Material * & _material;

  /**
   * Side Jacobian pre-multiplied by the weight.
   */
  const std::vector<Real> & _JxW;

  /**
   * Side shape function.
   */
  const std::vector<std::vector<Real> > & _phi;

  /**
   * Gradient of side shape function.
   */
  const std::vector<std::vector<RealGradient> > & _dphi;

  /**
   * Second derivative of side shape function.
   */
  const std::vector<std::vector<RealTensor> > & _d2phi;

  /**
   * Normal vectors at the quadrature points.
   */
  const std::vector<Point>& _normals;

  /**
   * Boundary quadrature rule.
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
   * Holds the current solution at the current quadrature point on the face.
   */
  MooseArray<Real> & _u;

  /**
   * Holds the current solution gradient at the current quadrature point on the face.
   */
  MooseArray<RealGradient> & _grad_u;

  /**
   * Holds the current solution second derivative at the current quadrature point on the face
   */
  MooseArray<RealTensor> & _second_u;

  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<Real> & coupledVal(std::string name);  

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<RealGradient> & coupledGrad(std::string name);

  /**
   * Just here for convenience.  Used in constructors... usually to deal with multiple dimensional stuff.
   */
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;

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

private:
  
  /**
   * Constructor helper, adds all var_nums associated with this
   * BC to the appropriate list.
   */
  void addVarNums(std::vector<unsigned int> & var_nums);
};

#endif //BOUNDARYCONDITION_H
