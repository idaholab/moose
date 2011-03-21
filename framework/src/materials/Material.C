#include "Material.h"
#include "Problem.h"
#include "SubProblem.h"
#include "MaterialData.h"

// system includes
#include <iostream>

template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::vector<unsigned int> >("block", "The id of the block (subdomain) that this material represents.");

  params.addPrivateParam<std::string>("built_by_action", "add_material");
  return params;
}


Material::Material(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters, false),
    TransientInterface(parameters),
    PostprocessorInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblemInterface *>("_subproblem")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _bnd(parameters.get<bool>("_bnd")),
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _qrule(_bnd ? _subproblem.qRuleFace(_tid) : _subproblem.qRule(_tid)),
    _JxW(_bnd ? _subproblem.JxWFace(_tid) : _subproblem.JxW(_tid)),
    _q_point(_bnd ? _subproblem.pointsFace(_tid) : _subproblem.points(_tid)),
    _current_elem(_subproblem.elem(_tid)),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _has_stateful_props(false),
    _block_id(parameters.get<unsigned int>("block_id")),

    _props(_material_data.props()),
    _props_old(_material_data.propsOld()),
    _props_older(_material_data.propsOlder()),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
  _props_elem       = new std::map<unsigned int, std::map<unsigned int, MaterialProperties> >;
  _props_elem_old   = new std::map<unsigned int, std::map<unsigned int, MaterialProperties> >;
  _props_elem_older = new std::map<unsigned int, std::map<unsigned int, MaterialProperties> >;

/*
  for (unsigned int i = 0; i < _coupled_to.size(); i++)
  {
    std::string coupled_var_name = _coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if (_moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getVariableNumber(coupled_var_name);
      _data._var_nums.insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (_moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getAuxVariableNumber(coupled_var_name);
      _data._aux_var_nums.insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
  */
}

Material::~Material()
{
  // TODO: Implement destructor to clean up after the _qp_prev and _qp_curr data objects
  
  //std::for_each(_qp_prev.begin(), _qp_prev.end(), DeleteFunctor());
  //std::for_each(_qp_curr.begin(), _qp_curr.end(), DeleteFunctor());

  {
    std::map<unsigned int, std::map<unsigned int, MaterialProperties> >::iterator i;
    for (i = _props_elem->begin(); i != _props_elem->end(); ++i)
    {
      std::map<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem;

    for (i = _props_elem_old->begin(); i != _props_elem_old->end(); ++i)
    {
      std::map<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem_old;

    for (i = _props_elem_older->begin(); i != _props_elem_older->end(); ++i)
    {
      std::map<unsigned int, MaterialProperties>::iterator j;
      for (j = i->second.begin(); j != i->second.end(); ++j)
      {
        MaterialProperties::iterator k;
        for (k = j->second.begin(); k != j->second.end(); ++k)
          delete k->second;
      }
    }
    delete _props_elem_older;
  }
}

unsigned int
Material::blockID()
{
  return _block_id;
}

void shallowCopyData(const std::set<std::string> & names,
                     MaterialProperties & data,
                     MaterialProperties & data_from)
{
  for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string name = *it;
    if (data[name] == NULL)
      data[name] = data_from[name]->init();
    data[name]->shallowCopy(data_from[name]);
  }
}

void
Material::reinit()
{
  unsigned int current_elem = _current_elem->id();
  _n_qpoints = _qrule->n_points();

  if (_has_stateful_props)
  {
    // initialize elemental data
    for (std::set<std::string>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
    {
      std::string name = *it;

      if ((*_props_elem)[current_elem][0][name] == NULL) (*_props_elem)[current_elem][0][name] = _props[name]->init();
      if ((*_props_elem_old)[current_elem][0][name] == NULL) (*_props_elem_old)[current_elem][0][name] = _props[name]->init();
      if ((*_props_elem_older)[current_elem][0][name] == NULL) (*_props_elem_older)[current_elem][0][name] = _props[name]->init();
    }

    shallowCopyData(_stateful_props, _props, (*_props_elem)[current_elem][0]);
    shallowCopyData(_stateful_props, _props_old, (*_props_elem_old)[current_elem][0]);
    shallowCopyData(_stateful_props, _props_older, (*_props_elem_older)[current_elem][0]);
  }

  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it)
  {
    mooseAssert(it->second != NULL, "Internal error in Material::materialReinit");
    it->second->resize(_n_qpoints);
  }

  if (_has_stateful_props)
  {
    for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      it->second->resize(_n_qpoints);
    for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
      it->second->resize(_n_qpoints);
  }

  computeProperties();

  if (_has_stateful_props)
  {
    shallowCopyData(_stateful_props, (*_props_elem)[current_elem][0], _props);
    shallowCopyData(_stateful_props, (*_props_elem_old)[current_elem][0], _props_old);
    shallowCopyData(_stateful_props, (*_props_elem_older)[current_elem][0], _props_older);
  }
}

void
Material::reinit(unsigned int side)
{
  unsigned int current_elem = _current_elem->id();
  _n_qpoints = _qrule->n_points();

  if (_has_stateful_props)
  {
    // initialize elemental data
    for (std::set<std::string>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
    {
      std::string name = *it;

      if ((*_props_elem)[current_elem][side][name] == NULL) (*_props_elem)[current_elem][side][name] = _props[name]->init();
      if ((*_props_elem_old)[current_elem][side][name] == NULL) (*_props_elem_old)[current_elem][side][name] = _props[name]->init();
      if ((*_props_elem_older)[current_elem][side][name] == NULL) (*_props_elem_older)[current_elem][side][name] = _props[name]->init();
    }

    shallowCopyData(_stateful_props, _props, (*_props_elem)[current_elem][side]);
    shallowCopyData(_stateful_props, _props_old, (*_props_elem_old)[current_elem][side]);
    shallowCopyData(_stateful_props, _props_older, (*_props_elem_older)[current_elem][side]);
  }

  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it)
  {
    mooseAssert(it->second != NULL, "Internal error in Material::materialReinit");
    it->second->resize(_n_qpoints);
  }

  if (_has_stateful_props)
  {
    for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      it->second->resize(_n_qpoints);
    for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
      it->second->resize(_n_qpoints);
  }
  
  computeProperties();

  if (_has_stateful_props)
  {
    shallowCopyData(_stateful_props, (*_props_elem)[current_elem][side], _props);
    shallowCopyData(_stateful_props, (*_props_elem_old)[current_elem][side], _props_old);
    shallowCopyData(_stateful_props, (*_props_elem_older)[current_elem][side], _props_older);
  }
}

bool
Material::hasStatefulProperties()
{
  return _has_stateful_props;
}

void
Material::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}

void
Material::computeQpProperties()
{
}

/**
 * Updates the old (first) material properties to the current/new material properties (second)
 */
void
Material::updateDataState()
{
  if (_qp_prev.size() != _qp_curr.size()) throw std::out_of_range("_qp_prev != _qp_curr");

  std::map<unsigned int, std::vector<QpData *> >::iterator i_prev = _qp_prev.begin();
  std::vector<QpData *>::iterator j_prev, j_curr;

  while (i_prev != _qp_prev.end())
    for (j_prev = i_prev->second.begin(), j_curr = i_prev->second.begin();
         j_prev != i_prev->second.end();
         ++j_prev, ++j_curr)
      *j_prev = *j_curr;

  if (_has_stateful_props)
  {
    std::swap(_props_elem_old,  _props_elem_older);   // Swap old and older
    std::swap(_props_elem, _props_elem_old);     // Swap current and "older" (which is now in old)
  }
}

void
Material::timeStepSetup()
{}

QpData *
Material::createData()
{
  return NULL;
}

std::vector<QpData *> &
Material::getData(QP_Data_Type qp_data_type)
{
  std::map<unsigned int, std::vector<QpData *> > *locMap;
  std::map<unsigned int, std::vector<QpData *> >::iterator i_map;
  unsigned int elemId = _current_elem->id();

  switch (qp_data_type)
  {
  case CURR:
    locMap = &_qp_curr;
    break;
  case PREV:
    locMap = &_qp_prev;
    break;
  }

  i_map = locMap->find(elemId);
  if (i_map != locMap->end())
    return i_map->second;
  else
  {
    // If the vector doesn't already exist create it
    std::vector<QpData *> *v;
    v = new std::vector<QpData *>(_qrule->n_points());
    std::vector<QpData *>::iterator i_vec = v->begin();

    for (;i_vec != v->end();++i_vec)
      *i_vec = createData();

    (*locMap)[elemId] = *v;

    return *v;
  }
}
