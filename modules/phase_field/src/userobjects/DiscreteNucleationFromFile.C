//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationFromFile.h"
#include "MooseMesh.h"

#include <algorithm>

registerMooseObject("PhaseFieldApp", DiscreteNucleationFromFile);

template <>
InputParameters
validParams<DiscreteNucleationFromFile>()
{
  InputParameters params = validParams<DiscreteNucleationInserterBase>();
  params.addClassDescription(
      "Manages the list of currently active nucleation sites and adds new "
      "sites according to a predetermined list from a CSV file (use this with sync_times).");
  params.addRequiredParam<FileName>(
      "file", "CSV file with (time, x, y, z) coordinates for nucleation events.");
  params.addRequiredParam<Real>("hold_time", "Time to keep each nucleus active");
  params.addParam<Real>("tolerance", 1e-9, "Tolerance for determining insertion time");
  return params;
}

DiscreteNucleationFromFile::DiscreteNucleationFromFile(const InputParameters & parameters)
  : DiscreteNucleationInserterBase(parameters),
    _hold_time(getParam<Real>("hold_time")),
    _reader(getParam<FileName>("file")),
    _history_pointer(0),
    _tol(getParam<Real>("tolerance")),
    _nucleation_rate(0.0)
{
  _reader.read();

  auto & names = _reader.getNames();
  auto & data = _reader.getData();

  const std::size_t rows = data[0].size();
  _nucleation_history.resize(rows);

  bool found_time = false;
  bool found_x = false;
  bool found_y = false;
  bool found_z = false;

  for (std::size_t i = 0; i < names.size(); ++i)
  {
    // make sure all data columns have the same length
    if (data[i].size() != rows)
      paramError("file", "Mismatching column lengths in file");

    if (names[i] == "time")
    {
      for (std::size_t j = 0; j < rows; ++j)
        _nucleation_history[j].first = data[i][j];
      found_time = true;
    }
    else if (names[i] == "x")
    {
      for (std::size_t j = 0; j < rows; ++j)
        _nucleation_history[j].second(0) = data[i][j];
      found_x = true;
    }
    else if (names[i] == "y")
    {
      for (std::size_t j = 0; j < rows; ++j)
        _nucleation_history[j].second(1) = data[i][j];
      found_y = true;
    }
    else if (names[i] == "z")
    {
      for (std::size_t j = 0; j < rows; ++j)
        _nucleation_history[j].second(2) = data[i][j];
      found_z = true;
    }
  }

  // check if all required columns were found
  if (!found_time)
    paramError("file", "Missing 'time' column in file");
  if (!found_x)
    paramError("file", "Missing 'x' column in file");
  if (!found_y && _mesh.dimension() >= 2)
    paramError("file", "Missing 'y' column in file");
  if (!found_z && _mesh.dimension() >= 3)
    paramError("file", "Missing 'z' column in file");

  // sort the nucleation history primarily according to time
  std::sort(_nucleation_history.begin(), _nucleation_history.end());
}

void
DiscreteNucleationFromFile::initialize()
{
  // clear insertion and deletion counter
  _changes_made = {0, 0};

  // expire entries from the local nucleus list (if the current time step converged)
  if (_fe_problem.converged())
  {
    unsigned int i = 0;
    while (i < _global_nucleus_list.size())
    {
      if (_global_nucleus_list[i].first - _tol <= _fe_problem.time())
      {
        // remove entry (by replacing with last element and shrinking size by one)
        _global_nucleus_list[i] = _global_nucleus_list.back();
        _global_nucleus_list.pop_back();
        _changes_made.second++;
      }
      else
        ++i;
    }
  }

  // check if it is time to insert from the nucleus history
  while (_history_pointer < _nucleation_history.size() &&
         _nucleation_history[_history_pointer].first <= _fe_problem.time())
  {
    _global_nucleus_list.push_back(
        NucleusLocation(_nucleation_history[_history_pointer].first + _hold_time,
                        _nucleation_history[_history_pointer].second));
    _changes_made.first++;
    _history_pointer++;
  }
}

void
DiscreteNucleationFromFile::finalize()
{
  // no communication necessary as all ranks have the full nucleus history from the
  // DelimitedFileReader
  _update_required = _changes_made.first > 0 || _changes_made.second > 0;
}
