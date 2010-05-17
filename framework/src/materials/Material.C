#include "Material.h"
#include "MooseSystem.h"
#include <iostream>

template<>
InputParameters validParams<Material>()
{
  InputParameters params;
  params.addRequiredParam<std::vector<unsigned int> >("block", "The id of the block (subdomain) that this material represents.");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Material is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Material which correspond with the coupled_as names");
  return params;
}

Material::Material(std::string name, MooseSystem & moose_system, InputParameters parameters) :
   _name(name),
   _moose_system(moose_system),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _dim(_moose_system._dim),
   _t(_moose_system._t),
   _dt(_moose_system._dt),
   _dt_old(_moose_system._dt_old),
   _is_transient(_moose_system._is_transient),
   _current_elem(_moose_system._current_elem[_tid]),
   _qrule(_moose_system._qrule[_tid]),
   _coupled_to(parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0)),
   _coupled_as(parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _real_zero(_moose_system._real_zero[_tid]),
   _zero(_moose_system._zero[_tid]),
   _grad_zero(_moose_system._grad_zero[_tid]),
   _second_zero(_moose_system._second_zero[_tid])

{
  // FIXME: this for statement will go into a common ancestor
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system._system->has_variable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);

      _coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_moose_system._var_nums.begin(),_moose_system._var_nums.end(),coupled_var_num) == _moose_system._var_nums.end())
        _moose_system._var_nums.push_back(coupled_var_num);

      if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
        _coupled_var_nums.push_back(coupled_var_num);
    }
    else //Look for it in the Aux system
    {
      unsigned int coupled_var_num = _moose_system._aux_system->variable_number(coupled_var_name);

      _aux_coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_moose_system._aux_var_nums.begin(),_moose_system._aux_var_nums.end(),coupled_var_num) == _moose_system._aux_var_nums.end())
        _moose_system._aux_var_nums.push_back(coupled_var_num);

      if(std::find(_aux_coupled_var_nums.begin(),_aux_coupled_var_nums.end(),coupled_var_num) == _aux_coupled_var_nums.end())
        _aux_coupled_var_nums.push_back(coupled_var_num);
    }
  }
}

/*
unsigned int
Material::blockID()
{
  return _block_id;
}
*/

void
Material::materialReinit()
{
  std::map<std::string, std::vector<Real> >::iterator it = _real_props.begin();
  std::map<std::string, std::vector<Real> >::iterator it_end = _real_props.end();

  for(;it!=it_end;++it)
    it->second.resize(_moose_system._qrule[_tid]->n_points(),1);

  std::map<std::string, std::vector<RealGradient> >::iterator grad_it = _gradient_props.begin();
  std::map<std::string, std::vector<RealGradient> >::iterator grad_it_end = _gradient_props.end();

  for(;grad_it!=grad_it_end;++grad_it)
    grad_it->second.resize(_moose_system._qrule[_tid]->n_points());


  std::map<std::string, std::vector<RealVectorValue> >::iterator real_vector_value_it = _real_vector_value_props.begin();
  std::map<std::string, std::vector<RealVectorValue> >::iterator real_vector_value_end = _real_vector_value_props.end();

  for(;real_vector_value_it!=real_vector_value_end;++real_vector_value_it)
    real_vector_value_it->second.resize(_moose_system._qrule[_tid]->n_points());

  std::map<std::string, std::vector<RealTensorValue> >::iterator tensor_it = _tensor_props.begin();
  std::map<std::string, std::vector<RealTensorValue> >::iterator tensor_it_end = _tensor_props.end();

  for(;tensor_it!=tensor_it_end;++tensor_it)
    tensor_it->second.resize(_moose_system._qrule[_tid]->n_points());

  std::map<std::string, std::vector<ColumnMajorMatrix> >::iterator column_major_matrix_it = _column_major_matrix_props.begin();
  std::map<std::string, std::vector<ColumnMajorMatrix> >::iterator column_major_matrix_it_end = _column_major_matrix_props.end();

  for(;column_major_matrix_it!=column_major_matrix_it_end;++column_major_matrix_it)
    column_major_matrix_it->second.resize(_moose_system._qrule[_tid]->n_points());

  computeProperties();
}

Real &
Material::getConstantRealProperty(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props.find(name);

  if(it != _constant_real_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<Real> &
Material::getRealProperty(const std::string & name)
{
  std::map<std::string, std::vector<Real> >::iterator it = _real_props.find(name);

  if(it != _real_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<RealGradient> &
Material::getGradientProperty(const std::string & name)
{
  std::map<std::string, std::vector<RealGradient> >::iterator it = _gradient_props.find(name);

  if(it != _gradient_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<RealVectorValue> &
Material::getRealVectorValueProperty(const std::string & name)
{
  std::map<std::string, std::vector<RealVectorValue> >::iterator it = _real_vector_value_props.find(name);
  
  if(it != _real_vector_value_props.end())
    return it->second;
  
  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<std::vector<Real> > &
Material::getVectorProperty(const std::string & name)
{
  std::map<std::string, std::vector<std::vector<Real> > >::iterator it = _vector_props.find(name);

  if(it != _vector_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<RealTensorValue> &
Material::getTensorProperty(const std::string & name)
{
  std::map<std::string, std::vector<RealTensorValue> >::iterator it = _tensor_props.find(name);

  if(it != _tensor_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<ColumnMajorMatrix> &
Material::getColumnMajorMatrixProperty(const std::string & name)
{
  std::map<std::string, std::vector<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props.find(name);

  if(it != _column_major_matrix_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

std::vector<std::vector<std::vector<Real> > > &
Material::getMatrixProperty(const std::string & name)
{
  std::map<std::string, std::vector<std::vector<std::vector<Real> > > >::iterator it = _matrix_props.find(name);

  if(it != _matrix_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

/**
 * Updates the old (first) material properties to the current/new material properies (second)
 */
void
Material::updateDataState()
{
  if (_qp_prev.size() != _qp_curr.size()) throw std::out_of_range("_qp_prev != _qp_curr");

  std::map<unsigned int, std::vector<QpData *> >::iterator i_prev = _qp_prev.begin();
  std::map<unsigned int, std::vector<QpData *> >::iterator i_curr = _qp_curr.begin(); 
  std::vector<QpData *>::iterator j_prev, j_curr;

  while (i_prev != _qp_prev.end())
    for (j_prev = i_prev->second.begin(), j_curr = i_prev->second.begin(); 
         j_prev != i_prev->second.end(); 
         ++j_prev, ++j_curr)
      *j_prev = *j_curr;
}

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

void
Material::subdomainSetup()
{
}

std::vector<Real> &
Material::declareRealProperty(const std::string & name)
{
  return _real_props[name];
}

Real &
Material::declareConstantRealProperty(const std::string & name)
{
  return _constant_real_props[name];
}

std::vector<RealGradient> &
Material::declareGradientProperty(const std::string & name)
{
  return _gradient_props[name];
}

std::vector<RealVectorValue> &
Material::declareRealVectorValueProperty(const std::string & name)
{
  return _real_vector_value_props[name];
}

std::vector<std::vector<Real> > &
Material::declareVectorProperty(const std::string & name)
{
  return _vector_props[name];
}

std::vector<RealTensorValue> &
Material::declareTensorProperty(const std::string & name)
{
  return _tensor_props[name];
}

std::vector<ColumnMajorMatrix> &
Material::declareColumnMajorMatrixProperty(const std::string & name)
{
  return _column_major_matrix_props[name];
}

std::vector<std::vector<std::vector<Real> > > &
Material::declareMatrixProperty(const std::string & name)
{
  return _matrix_props[name];
}

bool
Material::isAux(std::string name)
{
  return _aux_coupled_as_to_var_num.find(name) != _aux_coupled_as_to_var_num.end();
}

bool
Material::isCoupled(std::string name)
{
  bool found = std::find(_coupled_as.begin(),_coupled_as.end(),name) != _coupled_as.end();

  //See if it's an Aux variable
  if(!found)
    found = isAux(name);

  return found;
}

unsigned int
Material::coupled(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _coupled_as_to_var_num[name];
  else
    return Kernel::modifiedAuxVarNum(_aux_coupled_as_to_var_num[name]);
}

std::vector<Real> &
Material::coupledVal(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _moose_system._var_vals[_tid][_coupled_as_to_var_num[name]];
  else
    return _moose_system._aux_var_vals[_tid][_aux_coupled_as_to_var_num[name]];
}

std::vector<RealGradient> &
Material::coupledGrad(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _moose_system._var_grads[_tid][_coupled_as_to_var_num[name]];
  else
    return _moose_system._aux_var_grads[_tid][_aux_coupled_as_to_var_num[name]];
}
