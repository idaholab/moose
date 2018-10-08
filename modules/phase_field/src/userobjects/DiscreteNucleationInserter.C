//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationInserter.h"
#include "libmesh/parallel_algebra.h"

#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationInserter);

template <>
InputParameters
validParams<DiscreteNucleationInserter>()
{
  InputParameters params = validParams<DiscreteNucleationInserterBase>();
  params.addClassDescription("Manages the list of currently active nucleation sites and adds new "
                             "sites according to a given probability function.");
  params.addRequiredParam<MaterialPropertyName>(
      "probability", "Probability density for inserting a discrete nucleus");
  params.addRequiredParam<Real>("hold_time", "Time to keep each nucleus active");
  return params;
}

DiscreteNucleationInserter::DiscreteNucleationInserter(const InputParameters & parameters)
  : DiscreteNucleationInserterBase(parameters),
    _probability(getMaterialProperty<Real>("probability")),
    _hold_time(getParam<Real>("hold_time")),
    _local_nucleus_list(declareRestartableData("local_nucleus_list", NucleusList(0)))
{
}

void
DiscreteNucleationInserter::initialize()
{
  // clear insertion and deletion counter
  _changes_made = {0, 0};

  // expire entries from the local nucleus list (if the current time step converged)
  if (_fe_problem.converged())
  {
    unsigned int i = 0;
    while (i < _local_nucleus_list.size())
    {
      if (_local_nucleus_list[i].first <= _fe_problem.time())
      {
        // remove entry (by replacing with last element and shrinking size by one)
        _local_nucleus_list[i] = _local_nucleus_list.back();
        _local_nucleus_list.pop_back();
        _changes_made.second++;
      }
      else
        ++i;
    }
  }

  // we reassemble this list at every time step
  _global_nucleus_list.clear();

  // clear total nucleation rate
  _nucleation_rate = 0.0;
}

void
DiscreteNucleationInserter::execute()
{
  // check each qp for potential nucleation
  // TODO: we might as well place the nuclei at random positions within the element...
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const Real rate = _probability[qp] * _JxW[qp] * _coord[qp];
    _nucleation_rate += rate;

    const Real random = getRandomReal();

    // We check the random number against the inverse of the zero probability.
    // for performance reasons we do a quick check against the linearized form of
    // that probability, which is always strictly larger than the actual probability.
    // The expression below should short circuit and the expensive exponential
    // should rarely get evaluated
    if (random < rate * _fe_problem.dt() && random < (1.0 - std::exp(-rate * _fe_problem.dt())))
    {
      _local_nucleus_list.push_back(NucleusLocation(_fe_problem.time() + _hold_time, _q_point[qp]));
      _changes_made.first++;
    }
  }
}

void
DiscreteNucleationInserter::threadJoin(const UserObject & y)
{
  // combine _local_nucleus_list entries from all threads on the current process
  const DiscreteNucleationInserter & uo = static_cast<const DiscreteNucleationInserter &>(y);
  _global_nucleus_list.insert(
      _global_nucleus_list.end(), uo._local_nucleus_list.begin(), uo._local_nucleus_list.end());

  // sum up insertion and deletion counts
  _changes_made.first += uo._changes_made.first;
  _changes_made.second += uo._changes_made.second;

  // integrate total nucleation rate
  _nucleation_rate += uo._nucleation_rate;
}

void
DiscreteNucleationInserter::finalize()
{
  // add the _local_nucleus_list of thread zero
  _global_nucleus_list.insert(
      _global_nucleus_list.end(), _local_nucleus_list.begin(), _local_nucleus_list.end());

  /**
   * Pack the _global_nucleus_list into a simple vector of Real.
   * libMesh's allgather does not portably work on the original
   * _global_nucleus_list data structure!
   */
  std::vector<Real> comm_buffer(_global_nucleus_list.size() * 4);
  for (unsigned i = 0; i < _global_nucleus_list.size(); ++i)
  {
    comm_buffer[i * 4 + 0] = _global_nucleus_list[i].first;
    comm_buffer[i * 4 + 1] = _global_nucleus_list[i].second(0);
    comm_buffer[i * 4 + 2] = _global_nucleus_list[i].second(1);
    comm_buffer[i * 4 + 3] = _global_nucleus_list[i].second(2);
  }

  // combine _global_nucleus_lists from all MPI ranks
  _communicator.allgather(comm_buffer);

  // unpack the gathered _global_nucleus_list
  unsigned int n = comm_buffer.size() / 4;
  mooseAssert(comm_buffer.size() % 4 == 0,
              "Communication buffer has an unexpected size (not divisible by 4)");
  _global_nucleus_list.resize(n);
  for (unsigned i = 0; i < n; ++i)
  {
    _global_nucleus_list[i].first = comm_buffer[i * 4 + 0];
    _global_nucleus_list[i].second(0) = comm_buffer[i * 4 + 1];
    _global_nucleus_list[i].second(1) = comm_buffer[i * 4 + 2];
    _global_nucleus_list[i].second(2) = comm_buffer[i * 4 + 3];
  }

  // get the global number of changes (i.e. changes to _global_nucleus_list)
  gatherSum(_changes_made.first);
  gatherSum(_changes_made.second);

  // gather the total nucleation rate
  gatherSum(_nucleation_rate);

  _update_required = _changes_made.first > 0 || _changes_made.second > 0;
}
