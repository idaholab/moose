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

#ifndef OUTPUTPROBLEM_H
#define OUTPUTPROBLEM_H

#include "FEProblem.h"
#include "Output.h"
// libMesh
#include "equation_systems.h"
#include "vector_value.h"
#include "mesh_function.h"

class MooseMesh;
class OutputProblem;

template<>
InputParameters validParams<OutputProblem>();

class OutputProblem : public Problem
{
public:
  OutputProblem(const std::string & name, InputParameters parameters);
  virtual ~OutputProblem();

  // Solve /////
  virtual void init();

  // Setup /////
  virtual void timestepSetup();

  // Output system /////

  virtual Output & out() { return _out; }       // NOTE: don't like this -> remove and replace with better design
  virtual void output(bool /*force*/ = false) { _out.output(); }
  void outputInitial(bool out_init) { _output_initial = out_init; }

  virtual void outputPps(const FormattedTable & table);
  virtual void outputInput();

protected:
  FEProblem & _mproblem;
  MooseMesh _mesh;
  EquationSystems _eq;
  Output _out;
  /// output initial condition if true
  bool _output_initial;
  std::vector<std::vector<MeshFunction *> > _mesh_functions;
  NumericVector<Number> * _serialized_solution;
};

#endif
