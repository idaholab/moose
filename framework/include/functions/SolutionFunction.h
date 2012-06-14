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
#include "exodusII_io.h"

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

class SolutionFunction : public Function
{
public:
  SolutionFunction(const std::string & name, InputParameters parameters);

  virtual ~SolutionFunction();

  virtual void timestepSetup();
  virtual Real value(Real t, const Point & p);

protected:
  enum SolutionFileType
  {
    UNDEFINED,
    XDA,
    EXODUSII
  };
  static SolutionFileType getSolutionFileType(const std::string filetype);
  void updateExodusTimeInterpolation(Real time);
  bool updateExodusBracketingTimeIndices(Real time);

  std::string _mesh_file;
  SolutionFileType _file_type;
  std::string _es_file;
  std::string _system_name;
  std::string _var_name;
  int _exodus_time_index;
  bool _interpolate_times;

  Mesh * _mesh;
  EquationSystems * _es;
  System * _system;
  MeshFunction * _mesh_function;


  ExodusII_IO *_exodusII_io;

  NumericVector<Number> * _serialized_solution;

  //These are only used for interpolation between two exodusII times
  EquationSystems * _es2;
  System * _system2;
  MeshFunction * _mesh_function2;
  NumericVector<Number> * _serialized_solution2;
  Real _old_time;
  Real _interpolation_factor;
  const std::vector<Real> * _exodus_times;
  int _exodus_index1;
  int _exodus_index2;

};

#endif //SOLUTIONFUNCTION_H
