#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

// local includes
#include "Moose.h"
#include "ValidParams.h"
#include "MooseArray.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

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
class BoundaryCondition :
  public PDEBase,
  protected MaterialPropertyInterface
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  BoundaryCondition(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~BoundaryCondition(){}

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

protected:
  
  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  /**
   * Convenience reference to the FaceData object inside of MooseSystem
   */
  FaceData & _face_data;

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
   * Normal vectors at the quadrature points.
   */
  const std::vector<Point>& _normals;

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
  MooseArray<Real> & coupledVal(std::string var_name, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   * 
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<RealGradient> & coupledGrad(std::string var_name, int i = 0);


  /** Side shape function. 
   */ 
  const std::vector<std::vector<Real> > & _test; 
 
  /** 
   * Gradient of side shape function. 
   */ 
  const std::vector<std::vector<RealGradient> > & _grad_test; 
 
  /** 
   * Second derivative of side shape function. 
   */ 
  const std::vector<std::vector<RealTensor> > & _second_test; 
};

#endif //BOUNDARYCONDITION_H
