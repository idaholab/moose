#include "MaterialData.h"

//Moose includes
#include "MooseSystem.h"

MaterialData::MaterialData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{}

Real &
MaterialData::getConstantRealProperty(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props.find(name);

  if(it != _constant_real_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<Real> &
MaterialData::getRealProperty(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props.find(name);

  if(it != _real_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
MaterialData::getGradientProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props.find(name);

  if(it != _gradient_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
MaterialData::getRealVectorValueProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props.find(name);
  
  if(it != _real_vector_value_props.end())
    return it->second;
  
  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
MaterialData::getVectorProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props.find(name);

  if(it != _vector_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
MaterialData::getTensorProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props.find(name);

  if(it != _tensor_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
MaterialData::getColumnMajorMatrixProperty(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props.find(name);

  if(it != _column_major_matrix_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialData::getMatrixProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props.find(name);

  if(it != _matrix_props.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}



Real &
MaterialData::getConstantRealPropertyOld(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props_old.find(name);

  if(it != _constant_real_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<Real> &
MaterialData::getRealPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props_old.find(name);

  if(it != _real_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
MaterialData::getGradientPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props_old.find(name);

  if(it != _gradient_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
MaterialData::getRealVectorValuePropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props_old.find(name);
  
  if(it != _real_vector_value_props_old.end())
    return it->second;
  
  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
MaterialData::getVectorPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props_old.find(name);

  if(it != _vector_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
MaterialData::getTensorPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props_old.find(name);

  if(it != _tensor_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
MaterialData::getColumnMajorMatrixPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props_old.find(name);

  if(it != _column_major_matrix_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialData::getMatrixPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props_old.find(name);

  if(it != _matrix_props_old.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}



Real &
MaterialData::getConstantRealPropertyOlder(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _constant_real_props_older.find(name);

  if(it != _constant_real_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<Real> &
MaterialData::getRealPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _real_props_older.find(name);

  if(it != _real_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealGradient> &
MaterialData::getGradientPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _gradient_props_older.find(name);

  if(it != _gradient_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealVectorValue> &
MaterialData::getRealVectorValuePropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _real_vector_value_props_older.find(name);
  
  if(it != _real_vector_value_props_older.end())
    return it->second;
  
  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<Real> > &
MaterialData::getVectorPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _vector_props_older.find(name);

  if(it != _vector_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<RealTensorValue> &
MaterialData::getTensorPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _tensor_props_older.find(name);

  if(it != _tensor_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<ColumnMajorMatrix> &
MaterialData::getColumnMajorMatrixPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _column_major_matrix_props_older.find(name);

  if(it != _column_major_matrix_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialData::getMatrixPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _matrix_props_older.find(name);

  if(it != _matrix_props_older.end())
    return it->second;

  std::cerr<<"Material has no property named: "<<name;
  mooseError("");
}
