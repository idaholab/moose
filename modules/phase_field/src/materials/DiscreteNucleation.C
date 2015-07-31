#include "DiscreteNucleation.h"

template<>
InputParameters validParams<DiscreteNucleation>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Free energy contribution for nucleating discrete particles");
  params.addRequiredCoupledVar("op_names", "List of variables to force to a target concentration value");
  params.addRequiredParam<std::vector<Real> >("op_values", "List of target concentration values");
  params.addRequiredParam<MaterialPropertyName>("probability", "Probability density for inserting a discrete nucleus");
  params.addRequiredRangeCheckedParam<Real>("hold_time", "hold_time>0", "Duration for applying the concentration forcing penalty for each nucleation event");
  params.addParam<Real>("penalty", 20.0, "Penalty factor for enforcing the target concentrations");
  params.addParam<dof_id_type>("test", "Insert a fixed nucleus at a given element id at qp zero");
  return params;
}

struct
DiscreteNucleation::NucleationEvent
{
  NucleationEvent(Real insertion_time) : _insertion_time(insertion_time) {}
  Real _insertion_time;
};

DiscreteNucleation::DiscreteNucleation(const InputParameters & params) :
    DerivativeFunctionMaterialBase(params),
    _nvar(coupledComponents("op_names")),
    _op_index(_nvar),
    _op_values(getParam<std::vector<Real> >("op_values")),
    _probability(getMaterialProperty<Real>("probability")),
    _hold_time(getParam<Real>("hold_time")),
    _penalty(getParam<Real>("penalty"))
{
  // check inputs
  if (_nvar != _op_values.size())
    mooseError("The op_names and op_values parameter vectors must have the same number of entries");
  if (_nvar != _args.size())
    mooseError("Internal error.");

  // get libMesh variable numbers
  for (unsigned int i = 0; i < _nvar; ++i)
    _op_index[i] = argIndex(coupled("op_names", i));

  // debugging code
  if (isParamValid("test"))
  {
    std::vector<NucleationEvent*> dummy(_fe_problem.getMaxQps(), NULL);
    dummy[0] = new NucleationEvent(0.0);
    _nucleation_events.insert(std::pair<dof_id_type, std::vector<NucleationEvent *> >(getParam<dof_id_type>("test"), dummy));
  }

  setRandomResetFrequency(EXEC_TIMESTEP);
}

DiscreteNucleation::~DiscreteNucleation()
{
  // TODO: iterate over map and delete remaining nuclei
}

void
DiscreteNucleation::computeProperties()
{
  // check if a nucleation event list is available for the current element
  dof_id_type id = _current_elem->id();
  NucleationEventMap::iterator ne = _nucleation_events.find(id);

  // if this is a retry due to a cut timestep we need to drop all nuclei from the failed timestep
  if (_insertion_counter == 1 && ne != _nucleation_events.end())
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      // no nuclei with the current time as insertion time should exist yet, the nucleation stage is yet to come
      if (ne->second[_qp] != NULL && ne->second[_qp]->_insertion_time == _fe_problem.time())
      {
        delete ne->second[_qp];
        ne->second[_qp] = NULL;
      }

  // check if a nucleation event takes place (only do this _once_ per timestep)
  if (_insertion_counter == 1)
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (getRandomReal() < _probability[_qp] * _JxW[_qp] * _coord[_qp] * _fe_problem.dt())
      {
        _console << "nucleate! " << _fe_problem.comm().rank() << "\n";

        // add a map entry if necessary
        if (ne == _nucleation_events.end())
          ne = _nucleation_events.insert(std::pair<dof_id_type, std::vector<NucleationEvent *> >(id, std::vector<NucleationEvent*>(_qrule->n_points(), NULL))).first;

        // add event item
        if (ne->second[_qp] == NULL)
          ne->second[_qp] = new NucleationEvent(_fe_problem.time());
      }

      // List all nuclei at the beginning of a timestep
      if (ne != _nucleation_events.end() && ne->second[_qp] != NULL)
        _console << "Nucleus at " << id << " " << _qp << '\n';
    }

  // calculate penalty
  unsigned int ne_num = 0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // skip QPs without events
    bool nucleus = false;
    if (ne != _nucleation_events.end() && ne->second[_qp] != NULL)
    {
      if (ne->second[_qp]->_insertion_time + _hold_time > _fe_problem.time())
      {
        // use nucleus if it is not expred yet
        ne_num++;
        nucleus = true;
      }
      else
      {
        // it is expired, so delete it
        delete ne->second[_qp];
        ne->second[_qp] = NULL;
      }
    }

    // clear penalty value
    if (_prop_F)
      (*_prop_F)[_qp] = 0.0;

    for (unsigned int i = 0; i < _nvar; ++i)
    {
      const unsigned ii = _op_index[i];

      // sum up penalty contributions
      if (nucleus)
      {
        Real dc = (*_args[ii])[_qp] - _op_values[i];
        _console << (*_args[ii])[_qp] << " - " << _op_values[i] << '\n';
        if (_prop_F)
          (*_prop_F)[_qp] += dc * dc;

        // first derivative
        if (_prop_dF[ii])
          (*_prop_dF[ii])[_qp] = 2.0 * dc * _penalty;
      }
      else if (_prop_dF[ii])
        (*_prop_dF[ii])[_qp] = 0.0;

      // second derivatives
      for (unsigned int jj = ii; jj < _nvar; ++jj)
      {
        if (_prop_d2F[ii][jj])
          (*_prop_d2F[ii][jj])[_qp] = (!nucleus || ii != jj) ? 0.0 : 2.0 * _penalty;

        // third derivatives
        if (_third_derivatives)
          for (unsigned int kk = jj; kk < _nvar; ++kk)
            if (_prop_d3F[ii][jj][kk])
              (*_prop_d3F[ii][jj][kk])[_qp] = 0.0;
      }
    }

    // apply penalty factor
    if (nucleus && _prop_F)
      (*_prop_F)[_qp] *= _penalty;
  }

  // prune the map if no QPs here are
  if (ne != _nucleation_events.end() && ne_num == 0)
    _nucleation_events.erase(ne);
}
