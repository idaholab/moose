#ifndef DGKERNEL_H
#define DGKERNEL_H

// local includes
#include "Moose.h"
#include "ValidParams.h"
#include "MooseArray.h"
#include "BoundaryCondition.h"
#include "MaterialPropertyInterface.h"

//Forward Declarations
class DGKernel;
class MooseSystem;
class ElementData;

namespace libMesh
{
  class Elem;
  template<class T> class DenseMatrix;
}

template<>
InputParameters validParams<DGKernel>();

/** 
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 * 
 */
class DGKernel :
  public BoundaryCondition
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
  DGKernel(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~DGKernel();

protected:
  /**
   * The neighboring element
   */
  Elem * _neighbor_elem;

  /**
   * Convenience reference to the FaceData object for neighboring element inside of MooseSystem
   */
  DofData & _neighbor_dof_data;

  /**
   * Convenience reference to the FaceData object for neighboring element inside of MooseSystem
   */
  FaceData & _neighbor_face_data;

  /**
   *  Side shape function.
   */
  const std::vector<std::vector<Real> > & _test_neighbor;

  /**
   * Gradient of side shape function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_test_neighbor;

  /**
   * Second derivative of side shape function.
   */
  const std::vector<std::vector<RealTensor> > & _second_test_neighbor;

  /**
   * Holds the current solution at the current quadrature point.
   */
  MooseArray<Real> & _u_neighbor;

  /**
   * Holds the current solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u_neighbor;

  /**
   * Holds the current solution second derivative at the current quadrature point.
   */
  MooseArray<RealTensor> & _second_u_neighbor;

  /**
   * Holds the previous solution at the current quadrature point.
   */
  MooseArray<Real> & _u_old_neighbor;

  /**
   * Holds the t-2 solution at the current quadrature point.
   */
  MooseArray<Real> & _u_older_neighbor;

  /**
   * Holds the previous solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u_old_neighbor;

  /**
   * Holds the t-2 solution gradient at the current quadrature point.
   */
  MooseArray<RealGradient> & _grad_u_older_neighbor;
};

#endif //DGKERNEL_H
