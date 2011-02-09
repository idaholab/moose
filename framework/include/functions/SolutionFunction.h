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

#ifndef SOLUTIONFUNCTION_H
#define SOLUTIONFUNCTION_H

#include "Function.h"

// Forward Declarations
namespace libMesh
{
  class Mesh;
  class EquationSystems;
  class System;
  class MeshFunction;
  template<class T> class NumericVector;
}

class SolutionFunction;

template<>
InputParameters validParams<SolutionFunction>();

/**
 * Do nothing function
 */
class SolutionFunction : public Function
{
public:
  SolutionFunction(const std::string & name, InputParameters parameters);

  virtual ~SolutionFunction();

  virtual Real value(Real t, Real x, Real y = 0, Real z = 0);

protected:
  std::string _mesh_file;
  std::string _es_file;
  std::string _system_name;
  std::string _var_name;
  
  Mesh * _mesh;
  EquationSystems * _es;
  System * _system;
  MeshFunction * _mesh_function;

  NumericVector<Number> * _serialized_solution;
};

#endif //SOLUTIONFUNCTION_H
