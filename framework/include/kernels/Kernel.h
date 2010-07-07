#ifndef KERNEL_H
#define KERNEL_H

#include "dense_matrix.h"

// local includes
#include "Moose.h"
#include "ValidParams.h"
#include "MooseArray.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

// libmesh includes
#include "quadrature_gauss.h"

//Forward Declarations
class Material;
class Kernel;
class MooseSystem;
class ElementData;

namespace libMesh
{
  class Elem;
}


template<>
InputParameters validParams<Kernel>();

/** 
 * The Kernel class is responsible for calculating the residuals for various
 * physics.
 * 
 */
class Kernel :
  public PDEBase,
  protected MaterialPropertyInterface
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

protected:
  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

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
   * Holds the current solution at the current quadrature point.
   */
  MooseArray<Real> & _u;

  /**
   * The value of _u at a nodal position.  Used by non-integrated boundaries.
   */
  Real _u_node;

  /**
   * Holds the current solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u;

  /**
   * Holds the current solution second derivative at the current quadrature point.
   */
  MooseArray<RealTensor> & _second_u;

  /**
   * Holds the previous solution at the current quadrature point.
   */
  MooseArray<Real> & _u_old;

  /**
   * Holds the t-2 solution at the current quadrature point.
   */
  MooseArray<Real> & _u_older;

  /**
   * Holds the previous solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u_old;

  /**
   * Holds the t-2 solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u_older;

  /**
   * Current material
   */
  Material * & _material;
  
  /**
   * Interior test function.
   *
   * These are non-const so they can be modified for stabilization.
   */
  std::vector<std::vector<Real> > & _test;

  /**
   * Gradient of interior test function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_test;

  /**
   * Second derivative of interior test function.
   */
  const std::vector<std::vector<RealTensor> > & _second_test;
};

#endif //KERNEL_H
