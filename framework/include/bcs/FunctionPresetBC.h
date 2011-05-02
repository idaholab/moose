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

#ifndef FUNCTIONPRESETBC_H
#define FUNCTIONPRESETBC_H

#include "PresetNodalBC.h"

//Forward Declarations
class FunctionPresetBC;
class Function;

template<>
InputParameters validParams<FunctionPresetBC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionPresetBC : public PresetNodalBC
{
public:
  FunctionPresetBC(const std::string & name, InputParameters parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual Real computeQpValue();

  Function & _func;                             /// function being used for evaluation of this BC
};

#endif //FUNCTIONPRESETBC_H
