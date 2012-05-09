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

#ifndef NODALPOSTPROCESSOR_H
#define NODALPOSTPROCESSOR_H

#include "Postprocessor.h"
#include "Coupleable.h"
#include "UserObjectInterface.h"
#include "MooseVariableInterface.h"
#include "MooseVariable.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
// libMesh
#include "elem.h"

class MooseVariable;

//Forward Declarations
class NodalPostprocessor;

template<>
InputParameters validParams<NodalPostprocessor>();

class NodalPostprocessor :
  public Postprocessor,
  public Coupleable,
  public UserObjectInterface,
  public MooseVariableInterface,
  public TransientInterface,
  public MaterialPropertyInterface
{
public:
  NodalPostprocessor(const std::string & name, InputParameters parameters);

  const std::vector<std::string> & boundaries() { return _boundaries; }

protected:
  MooseVariable & _var;

  std::vector<std::string> _boundaries;

  const unsigned int _qp;
//  const std::vector< Point > & _q_point;
//  QBase * & _qrule;
//  const std::vector<Real> & _JxW;

  const Node * & _current_node;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
//  VariableValue & _u_old;                               ///< Holds the previous solution at the current quadrature point.
//  VariableValue & _u_older;                             ///< Holds the t-2 solution at the current quadrature point.

//  VariableGradient & _grad_u;                           ///< Holds the solution gradient at the current quadrature points

//  virtual Real doSomething();
};

#endif
