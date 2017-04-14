/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DiscreteNucleationInserter.h"
#include "libmesh/parallel_algebra.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DiscreteNucleationInserter>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Manages the list of currently active nucleation sites and adds new "
                             "sites according to a given probability function.");
  params.addRequiredParam<MaterialPropertyName>(
      "probability", "Probability density for inserting a discrete nucleus");
  params.addRequiredParam<Real>("hold_time", "Time to keep each nucleus active");
  params.addParam<Point>("test", "Insert a fixed nucleus at a point in the simulation cell");
  MooseUtils::setExecuteOnFlags(params, {EXEC_TIMESTEP_END});
  return params;
}

DiscreteNucleationInserter::DiscreteNucleationInserter(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _probability(getMaterialProperty<Real>("probability")),
    _hold_time(getParam<Real>("hold_time")),
    _changes_made(0),
    _global_nucleus_list(declareRestartableData("global_nucleus_list", NucleusList(0))),
    _local_nucleus_list(declareRestartableData("local_nucleus_list", NucleusList(0)))
{
  setRandomResetFrequency(EXEC_TIMESTEP_END);

  // debugging code (this will insert the entry into every processors list, but duplicate entries in
  // global should be OK)
  // we also assume that time starts at 0! But hey, this is only for debugging anyways...
  if (isParamValid("test"))
    _insert_test = true;
  else
    _insert_test = false;

  // force a map rebuild after restart or recover
  _changes_made = _app.isRecovering() || _app.isRestarting();
}

void
DiscreteNucleationInserter::initialize()
{
  _changes_made = 0;

  // insert test nucleus once
  if (_insert_test)
  {
    _local_nucleus_list.push_back(NucleusLocation(_hold_time, getParam<Point>("test")));
    _changes_made++;
    _insert_test = false;
  }

  // expire entries from the local nucleus list (if the current timestep converged)
  if (_fe_problem.converged())
  {
    unsigned int i = 0;
    while (i < _local_nucleus_list.size())
    {
      if (_local_nucleus_list[i].first + _fe_problem.dt() <= _fe_problem.time())
      {
        // remove entry (by replacing with last element and shrinking size by one)
        _local_nucleus_list[i] = _local_nucleus_list.back();
        _local_nucleus_list.pop_back();
        _changes_made++;
      }
      else
        ++i;
    }
  }

  // we reassemble this list at every timestep
  _global_nucleus_list.clear();
}

void
DiscreteNucleationInserter::execute()
{
  // check each qp for potential nucleation
  // TODO: we might as well place the nuclei at random positions within the element...
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    if (getRandomReal() < _probability[qp] * _JxW[qp] * _coord[qp] * _fe_problem.dt())
    {
      _local_nucleus_list.push_back(
          NucleusLocation(_fe_problem.dt() + _fe_problem.time() + _hold_time, _q_point[qp]));
      _changes_made++;
    }
}

void
DiscreteNucleationInserter::threadJoin(const UserObject & y)
{
  // combine _local_nucleus_list entries from all threads on the current process
  const DiscreteNucleationInserter & uo = static_cast<const DiscreteNucleationInserter &>(y);
  _global_nucleus_list.insert(
      _global_nucleus_list.end(), uo._local_nucleus_list.begin(), uo._local_nucleus_list.end());
  _changes_made += uo._changes_made;
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
  gatherSum(_changes_made);
}
