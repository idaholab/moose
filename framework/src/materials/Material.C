#include "Material.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "FaceData.h"
#include "MaterialData.h"
#include "QpData.h"
#include <iostream>

template<>
InputParameters validParams<Material>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::vector<unsigned int> >("block", "The id of the block (subdomain) that this material represents.");
  return params;
}

Material::Material(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  PDEBase(name, moose_system, parameters, moose_system.getQuadraturePointData(parameters.get<bool>("_is_boudary_material"))),
  _material_data(moose_system._material_data),
  _has_stateful_props(false),
  _props(_material_data._props),
  _props_old(_material_data._props_old),
  _props_older(_material_data._props_older)
{
  _props_elem = new std::map<unsigned int, std::map<std::string, PropertyValue *> >;
  _props_elem_old   = new std::map<unsigned int, std::map<std::string, PropertyValue *> >;
  _props_elem_older = new std::map<unsigned int, std::map<std::string, PropertyValue *> >;

  int bid = parameters.get<int>("_bid");

  for (unsigned int i = 0; i < _coupled_to.size(); i++)
  {
    std::string coupled_var_name = _coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if (_moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getVariableNumber(coupled_var_name);
      add_nonexistent(coupled_var_num, _data._var_nums[bid]);
    }
    //Look for it in the Aux system
    else if (_moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getAuxVariableNumber(coupled_var_name);
      add_nonexistent(coupled_var_num, _data._aux_var_nums[bid]);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

/*
unsigned int
Material::blockID()
{
  return _block_id;
}
*/

void shallowCopyData(const std::set<std::string> & names,
                     std::map<std::string, PropertyValue *> & data,
                     std::map<std::string, PropertyValue *> & data_from)
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
Material::materialReinit()
{
  unsigned int current_elem = _current_elem->id();

  _n_qpoints = _data._qrule[_tid]->n_points();

  if (_has_stateful_props)
  {
    // initialize elemental data
    for (std::set<std::string>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
    {
      std::string name = *it;
      if ((*_props_elem)[current_elem][name] == NULL) (*_props_elem)[current_elem][name] = _props[name]->init();
      if ((*_props_elem_old)[current_elem][name] == NULL) (*_props_elem_old)[current_elem][name] = _props[name]->init();
      if ((*_props_elem_older)[current_elem][name] == NULL) (*_props_elem_older)[current_elem][name] = _props[name]->init();
    }

    shallowCopyData(_stateful_props, _props, (*_props_elem)[current_elem]);
    shallowCopyData(_stateful_props, _props_old, (*_props_elem_old)[current_elem]);
    shallowCopyData(_stateful_props, _props_older, (*_props_elem_older)[current_elem]);
  }

  for (std::map<std::string, PropertyValue *>::iterator it = _props.begin(); it != _props.end(); ++it)
  {
    mooseAssert(it->second != NULL, "Internal error in Material::materialReinit");
    it->second->resize(_n_qpoints);
  }

  if (_has_stateful_props)
  {
    for (std::map<std::string, PropertyValue *>::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      it->second->resize(_n_qpoints);
    for (std::map<std::string, PropertyValue *>::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
      it->second->resize(_n_qpoints);
  }
  
  computeProperties();

  if (_has_stateful_props)
  {
    shallowCopyData(_stateful_props, (*_props_elem)[current_elem], _props);
    shallowCopyData(_stateful_props, (*_props_elem_old)[current_elem], _props_old);
    shallowCopyData(_stateful_props, (*_props_elem_older)[current_elem], _props_older);
  }
}

bool
Material::hasStatefulProperties()
{
  return _has_stateful_props;
}

/**
 * Updates the old (first) material properties to the current/new material properies (second)
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

Real
Material::computeQpResidual()
{
  return 0;
}
