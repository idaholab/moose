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

#ifndef DIRACKERNEL_H
#define DIRACKERNEL_H

#include <string>

//MOOSE includes
#include "Moose.h"
#include "DiracKernelData.h"
#include "DiracKernelInfo.h"
#include "MooseObject.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

//libMesh includes
#include "libmesh_common.h"
#include "ValidParams.h"

//Forward Declarations
class DiracKernel;
class MooseSystem;

template<>
InputParameters validParams<DiracKernel>();

/**
 * A DiracKernel is used when you need to add contributions to the residual by means of
 * multiplying some number by the shape functions on an element and adding the value into
 * the residual vector at the places associated with that shape function.
 *
 * This is common in point sources / sinks and various other algorithms.
 */
class DiracKernel : public PDEBase
{
public:
  DiracKernel(const std::string & name, MooseSystem &moose_system, InputParameters parameters);
  
  virtual ~DiracKernel(){}
  
  /** 
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /** 
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * This is where the DiracKernel should call addPoint() for each point it needs to have a
   * value distributed at.
   */
  virtual void addPoints() = 0;

  /** 
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual()=0;

  /** 
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian();

  /**
   * Whether or not this DiracKernel has something to distribute on this element.
   */
  bool hasPointsOnElem(const Elem * elem);

  /**
   * Whether or not this DiracKernel has something to distribute at this Point.
   */
  bool isActiveAtPoint(const Elem * elem, const Point & p);
  
protected:

  /**
   * Add the physical x,y,z point located in the element "elem" to the list of points
   * this DiracKernel will be asked to evaluate a value at.
   */
  void addPoint(const Elem * elem, Point p);

  /**
   * This is a highly inefficient way to add a point where this DiracKernel needs to be
   * evaluated.
   *
   * This spawns a search for the element containing that point!
   */
  void addPoint(Point p);
  
  DiracKernelData & _dirac_kernel_data;
  DiracKernelInfo & _dirac_kernel_info;

  /**
   * THe name of the variable this DiracKernel acts on.
   */
  std::string _var_name;

  /**
   * The list of elements that need distributions.
   */
  std::set<const Elem *> _elements;

  /**
   * The list of physical xyz Points that need to be evaluated in each element.
   */
  std::map<const Elem *, std::set<Point> > _points;

  ////////////////////////
  
  /**
   * Convenience reference to the DofData object inside of MooseSystem
   */
  DofData & _dof_data;

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
   * Time derivative of u
   */
  MooseArray<Real> & _u_dot;

  /**
   * Derivative of u_dot wrt u
   */
  MooseArray<Real> & _du_dot_du;

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

  /**
   * The points on the current element.
   */
  std::vector<Point> & _current_points;

  /**
   * The current point.
   */
  Point _current_point;
};
 
#endif
