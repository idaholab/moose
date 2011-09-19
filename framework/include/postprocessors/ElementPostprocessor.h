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

#ifndef ELEMENTPOSTPROCESSOR_H
#define ELEMENTPOSTPROCESSOR_H

#include "Postprocessor.h"
#include "Coupleable.h"
#include "MooseVariable.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
// libMesh
#include "elem.h"

class MooseVariable;

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor :
  public Postprocessor,
  public Coupleable,
  public TransientInterface,
  public MaterialPropertyInterface
{
public:
  ElementPostprocessor(const std::string & name, InputParameters parameters);

  unsigned int blockID() { return _block_id; }

  virtual Real computeIntegral();

protected:
  /**
   * The block ID this postprocessor works on
   */
  unsigned int _block_id;

  MooseVariable & _var;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  const Elem * & _current_elem;

  VariableValue & _u;                                   ///< Holds the solution at current quadrature points
  VariableValue & _u_old;                               ///< Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             ///< Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                           ///< Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u_old;                       ///< Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                     ///< Holds the t-2 solution gradient at the current quadrature point.

  virtual Real computeQpIntegral() = 0;
};

#endif
