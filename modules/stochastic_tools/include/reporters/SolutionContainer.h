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

class SolutionContainer : public GeneralReporter
{
public:
  static InputParameters validParams();
  SolutionContainer(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override;

  const std::vector<std::unique_ptr<NumericVector<Number>>> & getContainer()
  {
    return _accumulated_solutions;
  }

  unsigned int numLocalEntries() { return _accumulated_solutions.size(); }

protected:
  std::vector<std::unique_ptr<NumericVector<Number>>> & _accumulated_solutions;
};

// void dataStore(std::ostream & stream,
//                std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
//                void * context);
// void dataLoad(std::istream & stream,
//               std::map<VariableName, std::vector<std::unique_ptr<std::vector<Real>>>> & data,
//               void * context);
