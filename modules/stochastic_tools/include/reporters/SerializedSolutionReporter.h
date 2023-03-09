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
#include "libmesh/petsc_vector.h"

class SerializedSolutionReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  SerializedSolutionReporter(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Storage for the serialized solution vectors. The first vector is indexed by the given
  /// variable names. The second vector is the index of the solutions in case we need to store
  /// multiple solutions in a transient simulation. We store pointers to objects because we
  /// will frequently resize this container using push_back. The serialized solution is
  /// stored in standard vectors. We only serialize and store solutions on the ROOT processor.

  std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & _accumulated_solutions;

  std::vector<VariableName> _variable_names;
};

void dataStore(std::ostream & stream,
               std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
               void * context);
void dataLoad(std::istream & stream,
              std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
              void * context);
