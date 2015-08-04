/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiscreteNucleationMap.h"

template<>
InputParameters validParams<DiscreteNucleationMap>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<Real>("radius", 0.0, "Radius for the inserted nuclei");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  params.addCoupledVar("periodic", "Use the periodicity settings of this variable to populate the grain map");
  MultiMooseEnum setup_options(SetupInterface::getExecuteOptions());
  // the mapping needs to run at timestep begin, which is after the adaptivity
  // run of the previous timestep.
  setup_options = "timestep_begin";
  params.set<MultiMooseEnum>("execute_on") = setup_options;
  return params;
}

DiscreteNucleationMap::DiscreteNucleationMap(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _mesh_changed(false),
    _inserter(getUserObject<DiscreteNucleationInserter>("inserter")),
    _periodic(isCoupled("periodic") ? coupled("periodic") : -1),
    _radius(getParam<Real>("radius")),
    _nucleus_list(_inserter.getNucleusList())
{
}

void
DiscreteNucleationMap::initialize()
{
  if (_inserter.isMapUpdateRequired() || _mesh_changed)
  {
    _rebuild_map = true;
    _nucleus_map.clear();
  }
  else
    _rebuild_map = false;

  _mesh_changed = false;
  _zero_map.assign(_fe_problem.getMaxQps(), 0);
}

void
DiscreteNucleationMap::execute()
{
  if (_rebuild_map)
  {
    // reserve space for each quadrature point in the element
    _elem_map.resize(_qrule->n_points());

    // store a random number for each quadrature point
    unsigned int active_nuclei = 0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      for (unsigned i = 0; i < _nucleus_list.size(); ++i)
      {
        // use a non-periodic or periodic distance
        Real r = _periodic < 0 ?
                   (_q_point[qp] - _nucleus_list[i].second).size() :
                   _mesh.minPeriodicDistance(_periodic, _q_point[qp], _nucleus_list[i].second);
        if (r <= _radius)
        {
          _elem_map[qp] = 1;
          active_nuclei++;
        }
      }

    // if the map is not empty insert it
    if (active_nuclei > 0)
      _nucleus_map.insert(std::pair<dof_id_type, std::vector<char> >(_current_elem->id(), _elem_map));
  }
}

void
DiscreteNucleationMap::threadJoin(const UserObject &y)
{
  // if the map needs to be updated we merge the maps from all threads
  if (_rebuild_map)
  {
    const DiscreteNucleationMap & uo = static_cast<const DiscreteNucleationMap &>(y);
    _nucleus_map.insert(uo._nucleus_map.begin(), uo._nucleus_map.end());
  }
}

void
DiscreteNucleationMap::meshChanged()
{
  _mesh_changed = true;
}

const std::vector<char> &
DiscreteNucleationMap::nuclei(const Elem * elem) const
{
  NucleusMap::const_iterator i = _nucleus_map.find(elem->id());

  // if no entry in the map was found the element contains no nucleus
  if (i == _nucleus_map.end())
    return _zero_map;

  return i->second;
}
