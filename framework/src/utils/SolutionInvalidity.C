//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionInvalidity.h"

// MOOSE Includes
#include "MooseError.h"
#include "MooseApp.h"
#include "VariadicTable.h"

// System Includes
#include <chrono>
#include <memory>

SolutionInvalidity::SolutionInvalidity(MooseApp & app)
  : ConsoleStreamInterface(app),
    _solution_invalidity_registry(moose::internal::getSolutionInvalidityRegistry())
{
}

SolutionInvalidity::~SolutionInvalidity() {}
/// Count solution invalid occurrences for each solution id
void
SolutionInvalidity::setSolutionInvalid(SolutionID _solution_id)
{
  if (_solution_invalid_counts.size() <= _solution_id)
  {
    _solution_invalid_counts.resize(_solution_id + 1);
    ++_solution_invalid_counts[_solution_id];
  }
  else
  {
    ++_solution_invalid_counts[_solution_id];
  }
}

/// Loop over all the tracked objects and determine whether solution invalid is detected
bool
SolutionInvalidity::solutionInvalid() const
{
  unsigned int sum_of_elems = 0;
  std::for_each(_solution_invalid_counts.begin(),
                _solution_invalid_counts.end(),
                [&](int n) { sum_of_elems += n; });
  if (sum_of_elems < 1)
  {
    return false;
  }
  else
  {
    return true;
  }
}

/// Reset the number of solution invalid occurrences back to zero
void
SolutionInvalidity::resetSolutionInvalid()
{
  std::fill(_solution_invalid_counts.begin(), _solution_invalid_counts.end(), 0);
}

void
SolutionInvalidity::print(const ConsoleStream & console)
{
  console << "\nThe Summary Table of Solution Invalidity Occurences:\n";
  summaryTable().print(console);
}

/// Store all solution invalid warning for output
SolutionInvalidity::FullTable
SolutionInvalidity::summaryTable()
{
  FullTable vtable({"Section", "Calls"}, 2);

  vtable.setColumnFormat({
      VariadicTableColumnFormat::AUTO, // Section Name
      VariadicTableColumnFormat::AUTO, // Calls
  });

  vtable.setColumnPrecision({
      1, // Section Name
      0, // Calls
  });

  if (_solution_invalid_counts.size() > 0)
  {

    // Now print out the sections that contain solution invalid info and occurences
    for (unsigned int id = 0; id < _solution_invalid_counts.size(); id++)
    {

      vtable.addRow(_solution_invalidity_registry.sectionInfo(id)._name, // Section
                    _solution_invalid_counts[id]                         // Calls
      );
    }
  }

  return vtable;
}
