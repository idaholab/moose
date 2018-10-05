//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISCRETENUCLEATIONFROMFILE_H
#define DISCRETENUCLEATIONFROMFILE_H

#include "DiscreteNucleationInserterBase.h"
#include "DelimitedFileReader.h"

class DiscreteNucleationFromFile;

template <>
InputParameters validParams<DiscreteNucleationFromFile>();

/**
 * This UserObject manages the insertion and expiration of nuclei in the simulation
 * domain it manages a list of nuclei with their insertion times and their center
 * positions. A DiscreteNucleationMap is needed to enable the DiscreteNucleation
 * material to look up if a nucleus is present at a given element/qp.
 */
class DiscreteNucleationFromFile : public DiscreteNucleationInserterBase
{
public:
  DiscreteNucleationFromFile(const InputParameters & parameters);

  void initialize() override;
  void execute() override {}
  void threadJoin(const UserObject &) override {}
  void finalize() override;

  const Real & getRate() const override { return _nucleation_rate; }

protected:
  /// Duration of time each nucleus is kept active after insertion
  const Real _hold_time;

  /// CSV file to read
  MooseUtils::DelimitedFileReader _reader;

  /// Total nucleation history read from file
  NucleusList _nucleation_history;

  /// pointer to the next nucleation event in the history
  std::size_t _history_pointer;

  /// tolerance for determining insertion time
  const Real _tol;

  /// total nucleation rate
  const Real _nucleation_rate;
};

#endif // DISCRETENUCLEATIONFROMFILE_H
