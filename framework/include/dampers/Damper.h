/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DAMPER_H
#define DAMPER_H

// Moose Includes
#include "DamperData.h"
#include "PDEBase.h"
#include "MaterialPropertyInterface.h"

//Forward Declarations
class Damper;

template<>
InputParameters validParams<Damper>();

class Damper : protected PDEBase, protected MaterialPropertyInterface
{
public:
  Damper(std::string name, MooseSystem & moose_system, InputParameters parameters);

  /**
   * Computes this Damper's damping for one element.
   */
  Real computeDamping();

protected:
  /**
   * This MUST be overriden by a child damper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping() = 0;

  /**
   * Data associated just with dampers (ie the newton update).
   */
  DamperData & _damper_data;

  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  /**
   * The current newton increment.
   */
  MooseArray<Real> & _u_increment;

  /**
   * Holds the current solution at the current quadrature point.
   */
  MooseArray<Real> & _u;

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
};
 
#endif
