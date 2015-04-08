/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSSTAGNATIONBC_H
#define NSSTAGNATIONBC_H

#include "NodalBC.h"

// Forward Declarations
class NSStagnationBC;


// Specialization required of all user-level Moose objects
template<>
InputParameters validParams<NSStagnationBC>();


/**
 * This is the base class for the "imposed stagnation" value boundary
 * conditions.  Derived classes impose specified stagnation pressure
 * and temperature BCs as Dirichlet terms in the governing equations.
 */
class NSStagnationBC : public NodalBC
{
public:
  // Constructor
  NSStagnationBC(const std::string & name, InputParameters parameters);

  // Destructor, better be virtual
  virtual ~NSStagnationBC(){}

protected:

  /**
   * Must be implemented in derived class.
   */
  // virtual Real computeQpResidual();

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  VariableValue& _temperature;

  // Required paramters
  Real _gamma;
  Real _R;
};


#endif // NSSTAGNATIONBC_H
