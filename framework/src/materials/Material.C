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

Material::Material(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
//          parameters,
//          Kernel::_es->get_system(0).variable_name(0),
//          false,
//          parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0),
//          parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _moose_system(moose_system)
{}

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

