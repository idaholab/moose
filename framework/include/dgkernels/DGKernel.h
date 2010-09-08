/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DGKERNEL_H
#define DGKERNEL_H

// local includes
#include "Moose.h"
#include "ValidParams.h"
#include "MooseArray.h"
#include "PDEBase.h"
#include "FaceData.h"
#include "TwoMaterialPropertyInterface.h"

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
  public PDEBase,
  protected TwoMaterialPropertyInterface
{
protected:
    enum DGResidualType
    {
      Element,
      Neighbor
    };

    enum DGJacobianType
    {
      ElementElement,
      ElementNeighbor,
      NeighborElement,
      NeighborNeighbor
    };

public:

  /** 
   * Factory constructor initializes all internal references needed for residual computation.
   * 
   *
   * @param name The name of this kernel.
   * @param moose_system The moose_system this kernel is associated with
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  DGKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~DGKernel();

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

protected:
  /**
   * Convenience reference to the DofData object inside of MooseSystem
   */
  DofData & _dof_data;

  /**
   * Convenience reference to the FaceData object inside of MooseSystem
   */
  FaceData & _face_data;

  /**
   * Convenience reference to the FaceData object for neighboring element inside of MooseSystem
   */
  DofData & _neighbor_dof_data;

  /**
   * Convenience reference to the FaceData object for neighboring element inside of MooseSystem
   */
  FaceData & _neighbor_face_data;

  /**
   * Made-up number for internal edges
   */
  unsigned int _boundary_id;

  /**
   * The current side as an element
   * Only valid if there is a Dirichlet BC
   * on this side.
   */
  Elem * _side_elem;

  /**
   * The neighboring element
   */
  Elem * _neighbor_elem;

  /**
   * Current side.
   */
  unsigned int & _current_side;

  /**
   * Current side element.
   */
  const Elem * & _current_side_elem;

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

  /**
   * Normal vectors at the quadrature points.
   */
  const std::vector<Point>& _normals;

  /**
   *  Side shape function.
   */
  const std::vector<std::vector<Real> > & _phi_neighbor;

  /**
   * Gradient of side shape function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_phi_neighbor;

  /**
   * Second derivative of side shape function.
   */
  const std::vector<std::vector<RealTensor> > & _second_phi_neighbor;

  /**
   *  Side test function.
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

 /**
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(DGResidualType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(DGJacobianType type) = 0;


  VariableValue & coupledNeighborValue(std::string varname, int i);

  VariableGradient & coupledNeighborGradient(std::string varname, int i);

  VariableSecond & coupledNeighborSecond(std::string varname, int i);

  VariableValue & coupledNeighborValueOld(std::string varname, int i);

  VariableValue & coupledNeighborValueOlder(std::string varname, int i);

  VariableGradient & coupledNeighborGradientOld(std::string varname, int i);

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id)
  static const unsigned int InternalBndId;
};

#endif //DGKERNEL_H
