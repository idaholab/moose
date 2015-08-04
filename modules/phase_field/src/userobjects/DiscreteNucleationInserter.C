/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiscreteNucleationInserter.h"
#include "libmesh/parallel_algebra.h"

template<>
InputParameters validParams<DiscreteNucleationInserter>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<MaterialPropertyName>("probability", "Probability density for inserting a discrete nucleus");
  params.addRequiredParam<Real>("hold_time", "Time to keep each nucleus active");
  params.addParam<Point>("test", "Insert a fixed nucleus at a point in the simulation cell");
  MultiMooseEnum setup_options(SetupInterface::getExecuteOptions());
  setup_options = "timestep_end";
  params.set<MultiMooseEnum>("execute_on") = setup_options;
  return params;
}

DiscreteNucleationInserter::DiscreteNucleationInserter(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _probability(getMaterialProperty<Real>("probability")),
    _hold_time(getParam<Real>("hold_time")),
    _changes_made(0),
    _global_nucleus_list(0),
    _local_nucleus_list(0)
{
  setRandomResetFrequency(EXEC_TIMESTEP);

  // debugging code (this will insert the entry into every processors list, but duplicate entries in global should be OK)
  // we also assume that time starts at 0! But hey, this is only for debugging anyways...
  if (isParamValid("test"))
    _local_nucleus_list.push_back(NucleusLocation(_hold_time, getParam<Point>("test")));
}

void
DiscreteNucleationInserter::initialize()
{
  _changes_made = 0;

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
}

void
DiscreteNucleationInserter::execute()
{
  // check each qp for potential nucleation
  // TODO: we might as well place the nuclei at random positions within the element...
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    if (getRandomReal() < _probability[qp] * _JxW[qp] * _coord[qp] * _fe_problem.dt())
    {
      _local_nucleus_list.push_back(NucleusLocation(_fe_problem.dt() + _fe_problem.time() + _hold_time, _q_point[qp]));
      _changes_made++;
    }
}

void
DiscreteNucleationInserter::threadJoin(const UserObject &y)
{
  // combine _local_nucleus_list entries from all threads on the current process
  const DiscreteNucleationInserter & uo = static_cast<const DiscreteNucleationInserter &>(y);
  _local_nucleus_list.insert(_local_nucleus_list.end(), uo._local_nucleus_list.begin(), uo._local_nucleus_list.end());
  _changes_made += uo._changes_made;
}

void
DiscreteNucleationInserter::finalize()
{
  // here we need to combine all _local_nucleus_list into a the _global_nucleus_list
  _global_nucleus_list = _local_nucleus_list;
  _communicator.allgather(_global_nucleus_list);
  gatherSum(_changes_made);
}
