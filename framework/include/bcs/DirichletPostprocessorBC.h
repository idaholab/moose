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

#ifndef DIRICHLETPOSTPROCESSORBC_H
#define DIRICHLETPOSTPROCESSORBC_H

#include "NodalBC.h"


//Forward Declarations
class DirichletPostprocessorBC;

template<>
InputParameters validParams<DirichletPostprocessorBC>();

/**
 * Implements a simple constant Dirichlet BC where u is a postprocessor value on the boundary.
 */
class DirichletPostprocessorBC : public NodalBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DirichletPostprocessorBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  std::string _postprocessor_name;                              /// Value of u on the boundary (from postprocessor).
  Real & _value;                                                /// postprocessor value being set
};

#endif //DIRICHLETPOSTPROCESSORBC_H
