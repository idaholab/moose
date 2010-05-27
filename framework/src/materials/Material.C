#include "Material.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "MaterialData.h"
#include "QpData.h"
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
   _element_data(*moose_system._element_data),
   _material_data(*moose_system._material_data),
   _constant_real_props(_material_data._constant_real_props),
   _real_props(_material_data._real_props),
   _gradient_props(_material_data._gradient_props),
   _real_vector_value_props(_material_data._real_vector_value_props),
   _vector_props(_material_data._vector_props),
   _tensor_props(_material_data._tensor_props),
   _column_major_matrix_props(_material_data._column_major_matrix_props),
   _matrix_props(_material_data._matrix_props),                                                                                                                             _constant_real_props_old(_material_data._constant_real_props_old),                                                                                                                                  _real_props_old(_material_data._real_props_old),
   _gradient_props_old(_material_data._gradient_props_old),
   _real_vector_value_props_old(_material_data._real_vector_value_props_old),
   _vector_props_old(_material_data._vector_props_old),
   _tensor_props_old(_material_data._tensor_props_old),
   _column_major_matrix_props_old(_material_data._column_major_matrix_props_old),
   _matrix_props_old(_material_data._matrix_props_old),
   _constant_real_props_older(_material_data._constant_real_props_older),
   _real_props_older(_material_data._real_props_older),
   _gradient_props_older(_material_data._gradient_props_older),
   _real_vector_value_props_older(_material_data._real_vector_value_props_older),
   _vector_props_older(_material_data._vector_props_older),
   _tensor_props_older(_material_data._tensor_props_older),
   _column_major_matrix_props_older(_material_data._column_major_matrix_props_older),
   _matrix_props_older(_material_data._matrix_props_older),
   _tid(Moose::current_thread_id),
   _parameters(parameters),
   _dim(_moose_system._dim),
   _has_stateful_props(false),
   _t(_moose_system._t),
   _dt(_moose_system._dt),
   _dt_old(_moose_system._dt_old),
   _is_transient(_moose_system._is_transient),
   _current_elem(_element_data._current_elem[_tid]),
   _qrule(_element_data._qrule[_tid]),
   // _q_point is initialized to the first variables's associated fe_type (same physical space for all vars)
   _q_point(*(_element_data._q_point[_tid])[_element_data._dof_map->variable_type(0)]), 
   _coupled_to(parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0)),
   _coupled_as(parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0)),
   _real_zero(_moose_system._real_zero[_tid]),
   _zero(_moose_system._zero[_tid]),
   _grad_zero(_moose_system._grad_zero[_tid]),
   _second_zero(_moose_system._second_zero[_tid])

{
  _constant_real_props_current_elem       = new std::map<std::string, Real >;
  _real_props_current_elem                = new std::map<unsigned int, std::map<std::string, MooseArray<Real> > >;
  _gradient_props_current_elem            = new std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > >;
  _real_vector_value_props_current_elem   = new std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > >;
  _vector_props_current_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > >;
  _tensor_props_current_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > >;
  _column_major_matrix_props_current_elem = new std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > >;
  _matrix_props_current_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > >;

  _constant_real_props_old_elem       = new std::map<std::string, Real >;
  _real_props_old_elem                = new std::map<unsigned int, std::map<std::string, MooseArray<Real> > >;
  _gradient_props_old_elem            = new std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > >;
  _real_vector_value_props_old_elem   = new std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > >;
  _vector_props_old_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > >;
  _tensor_props_old_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > >;
  _column_major_matrix_props_old_elem = new std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > >;
  _matrix_props_old_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > >;

  _constant_real_props_older_elem       = new std::map<std::string, Real >;
  _real_props_older_elem                = new std::map<unsigned int, std::map<std::string, MooseArray<Real> > >;
  _gradient_props_older_elem            = new std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > >;
  _real_vector_value_props_older_elem   = new std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > >;
  _vector_props_older_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > >;
  _tensor_props_older_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > >;
  _column_major_matrix_props_older_elem = new std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > >;
  _matrix_props_older_elem              = new std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > >;

  
  // FIXME: this for statement will go into a common ancestor
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system._system->has_variable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system._system->variable_number(coupled_var_name);

      _coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_element_data._var_nums.begin(),_element_data._var_nums.end(),coupled_var_num) == _element_data._var_nums.end())
        _element_data._var_nums.push_back(coupled_var_num);

      if(std::find(_coupled_var_nums.begin(),_coupled_var_nums.end(),coupled_var_num) == _coupled_var_nums.end())
        _coupled_var_nums.push_back(coupled_var_num);
    }
    else //Look for it in the Aux system
    {
      unsigned int coupled_var_num = _moose_system._aux_system->variable_number(coupled_var_name);

      _aux_coupled_as_to_var_num[_coupled_as[i]] = coupled_var_num;

      if(std::find(_element_data._aux_var_nums.begin(),_element_data._aux_var_nums.end(),coupled_var_num) == _element_data._aux_var_nums.end())
        _element_data._aux_var_nums.push_back(coupled_var_num);

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

template<typename T>
void shallowCopyData(const std::vector<std::string> & names, T & data, T & data_from)
{
  std::vector<std::string>::const_iterator it = names.begin();
  std::vector<std::string>::const_iterator it_end = names.end();

  for(;it!=it_end;++it)
  {
    std::string name = *it;

    data[name].shallowCopy(data_from[name]);
  }
}

void
Material::materialReinit()
{
  unsigned int current_elem = _current_elem->id();

  unsigned int qpoints = _element_data._qrule[_tid]->n_points();

  if(_has_stateful_props)
  {
    // For constant properties
    {
      std::vector<std::string>::const_iterator it = _constant_real_stateful_props.begin();
      std::vector<std::string>::const_iterator it_end = _constant_real_stateful_props.end();

      for(;it!=it_end;++it)
      {
        std::string name = *it;

        _constant_real_props[name] = (*_constant_real_props_current_elem)[name];
        _constant_real_props_old[name] = (*_constant_real_props_old_elem)[name];
        _constant_real_props_older[name] = (*_constant_real_props_older_elem)[name];
      }
    }
    
    shallowCopyData(_real_stateful_props, _real_props, (*_real_props_current_elem)[current_elem]);
    shallowCopyData(_gradient_stateful_props, _gradient_props, (*_gradient_props_current_elem)[current_elem]);
    shallowCopyData(_real_vector_value_stateful_props, _real_vector_value_props, (*_real_vector_value_props_current_elem)[current_elem]);
    shallowCopyData(_vector_stateful_props, _vector_props, (*_vector_props_current_elem)[current_elem]);
    shallowCopyData(_tensor_stateful_props, _tensor_props, (*_tensor_props_current_elem)[current_elem]);
    shallowCopyData(_column_major_matrix_stateful_props, _column_major_matrix_props, (*_column_major_matrix_props_current_elem)[current_elem]);
    shallowCopyData(_matrix_stateful_props, _matrix_props, (*_matrix_props_current_elem)[current_elem]);

    shallowCopyData(_real_stateful_props, _real_props_old, (*_real_props_old_elem)[current_elem]);
    shallowCopyData(_gradient_stateful_props, _gradient_props_old, (*_gradient_props_old_elem)[current_elem]);
    shallowCopyData(_real_vector_value_stateful_props, _real_vector_value_props_old, (*_real_vector_value_props_old_elem)[current_elem]);
    shallowCopyData(_vector_stateful_props, _vector_props_old, (*_vector_props_old_elem)[current_elem]);
    shallowCopyData(_tensor_stateful_props, _tensor_props_old, (*_tensor_props_old_elem)[current_elem]);
    shallowCopyData(_column_major_matrix_stateful_props, _column_major_matrix_props_old, (*_column_major_matrix_props_old_elem)[current_elem]);
    shallowCopyData(_matrix_stateful_props, _matrix_props_old, (*_matrix_props_old_elem)[current_elem]);

    shallowCopyData(_real_stateful_props, _real_props_older, (*_real_props_older_elem)[current_elem]);
    shallowCopyData(_gradient_stateful_props, _gradient_props_older, (*_gradient_props_older_elem)[current_elem]);
    shallowCopyData(_real_vector_value_stateful_props, _real_vector_value_props_older, (*_real_vector_value_props_older_elem)[current_elem]);
    shallowCopyData(_vector_stateful_props, _vector_props_older, (*_vector_props_older_elem)[current_elem]);
    shallowCopyData(_tensor_stateful_props, _tensor_props_older, (*_tensor_props_older_elem)[current_elem]);
    shallowCopyData(_column_major_matrix_stateful_props, _column_major_matrix_props_older, (*_column_major_matrix_props_older_elem)[current_elem]);
    shallowCopyData(_matrix_stateful_props, _matrix_props_older, (*_matrix_props_older_elem)[current_elem]); 
  }
  
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props.begin();
  std::map<std::string, MooseArray<Real> >::iterator it_end = _real_props.end();

  for(;it!=it_end;++it)
    it->second.resize(qpoints);

  std::map<std::string, MooseArray<RealGradient> >::iterator grad_it = _gradient_props.begin();
  std::map<std::string, MooseArray<RealGradient> >::iterator grad_it_end = _gradient_props.end();

  for(;grad_it!=grad_it_end;++grad_it)
    grad_it->second.resize(qpoints);


  std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_it = _real_vector_value_props.begin();
  std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_end = _real_vector_value_props.end();

  for(;real_vector_value_it!=real_vector_value_end;++real_vector_value_it)
    real_vector_value_it->second.resize(qpoints);

  std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it = _tensor_props.begin();
  std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it_end = _tensor_props.end();

  for(;tensor_it!=tensor_it_end;++tensor_it)
    tensor_it->second.resize(qpoints);

  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it = _column_major_matrix_props.begin();
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it_end = _column_major_matrix_props.end();

  for(;column_major_matrix_it!=column_major_matrix_it_end;++column_major_matrix_it)
    column_major_matrix_it->second.resize(qpoints);

  if(_has_stateful_props)
  {
    {      
      std::map<std::string, MooseArray<Real> >::iterator it = _real_props_old.begin();
      std::map<std::string, MooseArray<Real> >::iterator it_end = _real_props_old.end();

      for(;it!=it_end;++it)
        it->second.resize(qpoints,0);

      std::map<std::string, MooseArray<RealGradient> >::iterator grad_it = _gradient_props_old.begin();
      std::map<std::string, MooseArray<RealGradient> >::iterator grad_it_end = _gradient_props_old.end();

      for(;grad_it!=grad_it_end;++grad_it)
        grad_it->second.resize(qpoints,0);


      std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_it = _real_vector_value_props_old.begin();
      std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_end = _real_vector_value_props_old.end();

      for(;real_vector_value_it!=real_vector_value_end;++real_vector_value_it)
        real_vector_value_it->second.resize(qpoints);

      std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it = _tensor_props_old.begin();
      std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it_end = _tensor_props_old.end();

      for(;tensor_it!=tensor_it_end;++tensor_it)
        tensor_it->second.resize(qpoints,0);

      std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it = _column_major_matrix_props_old.begin();
      std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it_end = _column_major_matrix_props_old.end();

      for(;column_major_matrix_it!=column_major_matrix_it_end;++column_major_matrix_it)
        column_major_matrix_it->second.resize(qpoints,0);
    }

    {      
      std::map<std::string, MooseArray<Real> >::iterator it = _real_props_older.begin();
      std::map<std::string, MooseArray<Real> >::iterator it_end = _real_props_older.end();

      for(;it!=it_end;++it)
        it->second.resize(qpoints,0);

      std::map<std::string, MooseArray<RealGradient> >::iterator grad_it = _gradient_props_older.begin();
      std::map<std::string, MooseArray<RealGradient> >::iterator grad_it_end = _gradient_props_older.end();

      for(;grad_it!=grad_it_end;++grad_it)
        grad_it->second.resize(qpoints,0);


      std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_it = _real_vector_value_props_older.begin();
      std::map<std::string, MooseArray<RealVectorValue> >::iterator real_vector_value_end = _real_vector_value_props_older.end();

      for(;real_vector_value_it!=real_vector_value_end;++real_vector_value_it)
        real_vector_value_it->second.resize(qpoints);

      std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it = _tensor_props_older.begin();
      std::map<std::string, MooseArray<RealTensorValue> >::iterator tensor_it_end = _tensor_props_older.end();

      for(;tensor_it!=tensor_it_end;++tensor_it)
        tensor_it->second.resize(qpoints,0);

      std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it = _column_major_matrix_props_older.begin();
      std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator column_major_matrix_it_end = _column_major_matrix_props_older.end();

      for(;column_major_matrix_it!=column_major_matrix_it_end;++column_major_matrix_it)
        column_major_matrix_it->second.resize(qpoints,0);
    }

  }
  
  computeProperties();

  if(_has_stateful_props)
  {
    // For constant properties
    {
      std::vector<std::string>::const_iterator it = _constant_real_stateful_props.begin();
      std::vector<std::string>::const_iterator it_end = _constant_real_stateful_props.end();

      for(;it!=it_end;++it)
      {
        std::string name = *it;

        (*_constant_real_props_current_elem)[name] = _constant_real_props[name];
        (*_constant_real_props_old_elem)[name] = _constant_real_props_old[name];
        (*_constant_real_props_older_elem)[name] = _constant_real_props_older[name];
      }
    }

    shallowCopyData(_real_stateful_props, (*_real_props_current_elem)[current_elem], _real_props);

//    if(current_elem == 1)
//      std::cout<<(*_real_props_current_elem)[current_elem]["test"][0]<<std::endl;
    
    shallowCopyData(_gradient_stateful_props, (*_gradient_props_current_elem)[current_elem], _gradient_props);
    shallowCopyData(_real_vector_value_stateful_props, (*_real_vector_value_props_current_elem)[current_elem], _real_vector_value_props);
    shallowCopyData(_vector_stateful_props, (*_vector_props_current_elem)[current_elem], _vector_props);
    shallowCopyData(_tensor_stateful_props, (*_tensor_props_current_elem)[current_elem], _tensor_props);
    shallowCopyData(_column_major_matrix_stateful_props, (*_column_major_matrix_props_current_elem)[current_elem], _column_major_matrix_props);
    shallowCopyData(_matrix_stateful_props, (*_matrix_props_current_elem)[current_elem], _matrix_props);

    shallowCopyData(_real_stateful_props, (*_real_props_old_elem)[current_elem], _real_props_old);
    shallowCopyData(_gradient_stateful_props, (*_gradient_props_old_elem)[current_elem], _gradient_props_old);
    shallowCopyData(_real_vector_value_stateful_props, (*_real_vector_value_props_old_elem)[current_elem], _real_vector_value_props_old);
    shallowCopyData(_vector_stateful_props, (*_vector_props_old_elem)[current_elem], _vector_props_old);
    shallowCopyData(_tensor_stateful_props, (*_tensor_props_old_elem)[current_elem], _tensor_props_old);
    shallowCopyData(_column_major_matrix_stateful_props, (*_column_major_matrix_props_old_elem)[current_elem], _column_major_matrix_props_old);
    shallowCopyData(_matrix_stateful_props, (*_matrix_props_old_elem)[current_elem], _matrix_props_old);

    shallowCopyData(_real_stateful_props, (*_real_props_older_elem)[current_elem], _real_props_older);
    shallowCopyData(_gradient_stateful_props, (*_gradient_props_older_elem)[current_elem], _gradient_props_older);
    shallowCopyData(_real_vector_value_stateful_props, (*_real_vector_value_props_older_elem)[current_elem], _real_vector_value_props_older);
    shallowCopyData(_vector_stateful_props, (*_vector_props_older_elem)[current_elem], _vector_props_older);
    shallowCopyData(_tensor_stateful_props, (*_tensor_props_older_elem)[current_elem], _tensor_props_older);
    shallowCopyData(_column_major_matrix_stateful_props, (*_column_major_matrix_props_older_elem)[current_elem], _column_major_matrix_props_older);
    shallowCopyData(_matrix_stateful_props, (*_matrix_props_older_elem)[current_elem], _matrix_props_older);
  }
}

bool
Material::hasStatefulProperties()
{
  return _has_stateful_props;
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

MooseArray<Real> &
Material::getRealProperty(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props.find(name);

  if(it != _real_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
Material::getGradientProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props.find(name);

  if(it != _gradient_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
Material::getRealVectorValueProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props.find(name);
  
  if(it != _real_vector_value_props.end())
    return it->second;
  
  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
Material::getVectorProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props.find(name);

  if(it != _vector_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
Material::getTensorProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props.find(name);

  if(it != _tensor_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
Material::getColumnMajorMatrixProperty(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props.find(name);

  if(it != _column_major_matrix_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::getMatrixProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props.find(name);

  if(it != _matrix_props.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}



Real &
Material::getConstantRealPropertyOld(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props_old.find(name);

  if(it != _constant_real_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<Real> &
Material::getRealPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props_old.find(name);

  if(it != _real_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
Material::getGradientPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props_old.find(name);

  if(it != _gradient_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
Material::getRealVectorValuePropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props_old.find(name);
  
  if(it != _real_vector_value_props_old.end())
    return it->second;
  
  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
Material::getVectorPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props_old.find(name);

  if(it != _vector_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
Material::getTensorPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props_old.find(name);

  if(it != _tensor_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
Material::getColumnMajorMatrixPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props_old.find(name);

  if(it != _column_major_matrix_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::getMatrixPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props_old.find(name);

  if(it != _matrix_props_old.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}



Real &
Material::getConstantRealPropertyOlder(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props_older.find(name);

  if(it != _constant_real_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<Real> &
Material::getRealPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props_older.find(name);

  if(it != _real_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
Material::getGradientPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props_older.find(name);

  if(it != _gradient_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
Material::getRealVectorValuePropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props_older.find(name);
  
  if(it != _real_vector_value_props_older.end())
    return it->second;
  
  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
Material::getVectorPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props_older.find(name);

  if(it != _vector_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
Material::getTensorPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props_older.find(name);

  if(it != _tensor_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
Material::getColumnMajorMatrixPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props_older.find(name);

  if(it != _column_major_matrix_props_older.end())
    return it->second;

  std::cerr<<"Material "<<_name<<" has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::getMatrixPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props_older.find(name);

  if(it != _matrix_props_older.end())
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

  if(_has_stateful_props)
  {
    // Swap old and older
    std::swap(_constant_real_props_old_elem       , _constant_real_props_older_elem);
    std::swap(_real_props_old_elem                , _real_props_older_elem);
    std::swap(_gradient_props_old_elem            , _gradient_props_older_elem);
    std::swap(_real_vector_value_props_old_elem   , _real_vector_value_props_older_elem);
    std::swap(_vector_props_old_elem              , _vector_props_older_elem);
    std::swap(_tensor_props_old_elem              , _tensor_props_older_elem);
    std::swap(_column_major_matrix_props_old_elem , _column_major_matrix_props_older_elem);
    std::swap(_matrix_props_old_elem              , _matrix_props_older_elem);

    // Swap current and "older" (which is now in old)
    std::swap(_constant_real_props_current_elem       , _constant_real_props_old_elem);
    std::swap(_real_props_current_elem                , _real_props_old_elem);
    std::swap(_gradient_props_current_elem            , _gradient_props_old_elem);
    std::swap(_real_vector_value_props_current_elem   , _real_vector_value_props_old_elem);
    std::swap(_vector_props_current_elem              , _vector_props_old_elem);
    std::swap(_tensor_props_current_elem              , _tensor_props_old_elem);
    std::swap(_column_major_matrix_props_current_elem , _column_major_matrix_props_old_elem);
    std::swap(_matrix_props_current_elem              , _matrix_props_old_elem);
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

void
Material::subdomainSetup()
{}

MooseArray<Real> &
Material::declareRealProperty(const std::string & name)
{
  return _real_props[name];
}

Real &
Material::declareConstantRealProperty(const std::string & name)
{
  return _constant_real_props[name];
}

MooseArray<RealGradient> &
Material::declareGradientProperty(const std::string & name)
{
  return _gradient_props[name];
}

MooseArray<RealVectorValue> &
Material::declareRealVectorValueProperty(const std::string & name)
{
  return _real_vector_value_props[name];
}

MooseArray<MooseArray<Real> > &
Material::declareVectorProperty(const std::string & name)
{
  return _vector_props[name];
}

MooseArray<RealTensorValue> &
Material::declareTensorProperty(const std::string & name)
{
  return _tensor_props[name];
}

MooseArray<ColumnMajorMatrix> &
Material::declareColumnMajorMatrixProperty(const std::string & name)
{
  return _column_major_matrix_props[name];
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::declareMatrixProperty(const std::string & name)
{
  return _matrix_props[name];
}




MooseArray<Real> &
Material::declareRealPropertyOld(const std::string & name)
{
  _has_stateful_props = true;
  
  if(std::find(_real_stateful_props.begin(), _real_stateful_props.end(), name) == _real_stateful_props.end())
    _real_stateful_props.push_back(name);
  
  return _real_props_old[name];
}

Real &
Material::declareConstantRealPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_constant_real_stateful_props.begin(), _constant_real_stateful_props.end(), name) == _constant_real_stateful_props.end())
    _constant_real_stateful_props.push_back(name);
  
  return _constant_real_props_old[name];
}

MooseArray<RealGradient> &
Material::declareGradientPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_gradient_stateful_props.begin(), _gradient_stateful_props.end(), name) == _gradient_stateful_props.end())
    _gradient_stateful_props.push_back(name);

  return _gradient_props_old[name];
}

MooseArray<RealVectorValue> &
Material::declareRealVectorValuePropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_real_vector_value_stateful_props.begin(), _real_vector_value_stateful_props.end(), name) == _real_vector_value_stateful_props.end())
    _real_vector_value_stateful_props.push_back(name);  
  
  return _real_vector_value_props_old[name];
}

MooseArray<MooseArray<Real> > &
Material::declareVectorPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_vector_stateful_props.begin(), _vector_stateful_props.end(), name) == _vector_stateful_props.end())
    _vector_stateful_props.push_back(name);

  return _vector_props_old[name];
}

MooseArray<RealTensorValue> &
Material::declareTensorPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_tensor_stateful_props.begin(), _tensor_stateful_props.end(), name) == _tensor_stateful_props.end())
    _tensor_stateful_props.push_back(name);

  return _tensor_props_old[name];
}

MooseArray<ColumnMajorMatrix> &
Material::declareColumnMajorMatrixPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_column_major_matrix_stateful_props.begin(), _column_major_matrix_stateful_props.end(), name) == _column_major_matrix_stateful_props.end())
    _column_major_matrix_stateful_props.push_back(name);

  return _column_major_matrix_props_old[name];
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::declareMatrixPropertyOld(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_matrix_stateful_props.begin(), _matrix_stateful_props.end(), name) == _matrix_stateful_props.end())
    _matrix_stateful_props.push_back(name);

  return _matrix_props_old[name];
}






MooseArray<Real> &
Material::declareRealPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;
  
  if(std::find(_real_stateful_props.begin(), _real_stateful_props.end(), name) == _real_stateful_props.end())
    _real_stateful_props.push_back(name);
  
  return _real_props_older[name];
}

Real &
Material::declareConstantRealPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_constant_real_stateful_props.begin(), _constant_real_stateful_props.end(), name) == _constant_real_stateful_props.end())
    _constant_real_stateful_props.push_back(name);
  
  return _constant_real_props_older[name];
}

MooseArray<RealGradient> &
Material::declareGradientPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_gradient_stateful_props.begin(), _gradient_stateful_props.end(), name) == _gradient_stateful_props.end())
    _gradient_stateful_props.push_back(name);

  return _gradient_props_older[name];
}

MooseArray<RealVectorValue> &
Material::declareRealVectorValuePropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_real_vector_value_stateful_props.begin(), _real_vector_value_stateful_props.end(), name) == _real_vector_value_stateful_props.end())
    _real_vector_value_stateful_props.push_back(name);  
  
  return _real_vector_value_props_older[name];
}

MooseArray<MooseArray<Real> > &
Material::declareVectorPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_vector_stateful_props.begin(), _vector_stateful_props.end(), name) == _vector_stateful_props.end())
    _vector_stateful_props.push_back(name);

  return _vector_props_older[name];
}

MooseArray<RealTensorValue> &
Material::declareTensorPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_tensor_stateful_props.begin(), _tensor_stateful_props.end(), name) == _tensor_stateful_props.end())
    _tensor_stateful_props.push_back(name);

  return _tensor_props_older[name];
}

MooseArray<ColumnMajorMatrix> &
Material::declareColumnMajorMatrixPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_column_major_matrix_stateful_props.begin(), _column_major_matrix_stateful_props.end(), name) == _column_major_matrix_stateful_props.end())
    _column_major_matrix_stateful_props.push_back(name);

  return _column_major_matrix_props_older[name];
}

MooseArray<MooseArray<MooseArray<Real> > > &
Material::declareMatrixPropertyOlder(const std::string & name)
{
  _has_stateful_props = true;

  if(std::find(_matrix_stateful_props.begin(), _matrix_stateful_props.end(), name) == _matrix_stateful_props.end())
    _matrix_stateful_props.push_back(name);

  return _matrix_props_older[name];
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

MooseArray<Real> &
Material::coupledVal(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_vals[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_vals[_tid][_aux_coupled_as_to_var_num[name]];
}

MooseArray<RealGradient> &
Material::coupledGrad(std::string name)
{
  if(!isCoupled(name))
  {
    std::cerr<<std::endl<<"Kernel "<<_name<<" was not provided with a variable coupled_as "<<name<<std::endl<<std::endl;
    mooseError("");
  }

  if(!isAux(name))
    return _element_data._var_grads[_tid][_coupled_as_to_var_num[name]];
  else
    return _element_data._aux_var_grads[_tid][_aux_coupled_as_to_var_num[name]];
}
