//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
class Transient;
// class NonlinearSystem;
// class AuxiliarySystem;
// class EquationSystems;

/**
 * Report the time and iteration information for the simulation.
 */
class MeshInfo : public GeneralReporter
{
public:
  static InputParameters validParams();
  MeshInfo(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  const MultiMooseEnum & _items;

  // Reporter values to return (all are computed as "distributed" values)
  unsigned int & _num_dofs;
  unsigned int & _num_dofs_nl;
  unsigned int & _num_dofs_aux;
  unsigned int & _num_elem;
  unsigned int & _num_node;

  // Used to allow for optional declare
  unsigned int _dummy_unsigned_int = 0;

  // Helper to perform optional declaration based on "_items"
  unsigned int & declareHelper(const std::string & item_name);

private:
  // const EquationSystems & _equation_systems;
  // const NonlinearSystem & _nonlinear_system;
  // const AuxiliarySystem & _aux_system;
};
