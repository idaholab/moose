#include "MaterialPropertyInterface.h"

#include "MooseSystem.h"

MaterialPropertyInterface::MaterialPropertyInterface(MaterialData & material_data):
  _material_data(material_data)
{}


Real &
MaterialPropertyInterface::getConstantRealMaterialProperty(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _material_data._constant_real_props.find(name);

  if(it != _material_data._constant_real_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<Real> &
MaterialPropertyInterface::getRealMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _material_data._real_props.find(name);

  if(it != _material_data._real_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealGradient> &
MaterialPropertyInterface::getGradientMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _material_data._gradient_props.find(name);

  if(it != _material_data._gradient_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealVectorValue> &
MaterialPropertyInterface::getRealVectorValueMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _material_data._real_vector_value_props.find(name);
  
  if(it != _material_data._real_vector_value_props.end())
    return it->second;
  
  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<Real> > &
MaterialPropertyInterface::getVectorMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _material_data._vector_props.find(name);

  if(it != _material_data._vector_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealTensorValue> &
MaterialPropertyInterface::getTensorMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _material_data._tensor_props.find(name);

  if(it != _material_data._tensor_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<ColumnMajorMatrix> &
MaterialPropertyInterface::getColumnMajorMatrixMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _material_data._column_major_matrix_props.find(name);

  if(it != _material_data._column_major_matrix_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialPropertyInterface::getMatrixMaterialProperty(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _material_data._matrix_props.find(name);

  if(it != _material_data._matrix_props.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}



Real &
MaterialPropertyInterface::getConstantRealMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _material_data._constant_real_props_old.find(name);

  if(it != _material_data._constant_real_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<Real> &
MaterialPropertyInterface::getRealMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _material_data._real_props_old.find(name);

  if(it != _material_data._real_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealGradient> &
MaterialPropertyInterface::getGradientMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _material_data._gradient_props_old.find(name);

  if(it != _material_data._gradient_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealVectorValue> &
MaterialPropertyInterface::getRealVectorValueMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _material_data._real_vector_value_props_old.find(name);
  
  if(it != _material_data._real_vector_value_props_old.end())
    return it->second;
  
  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<Real> > &
MaterialPropertyInterface::getVectorMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _material_data._vector_props_old.find(name);

  if(it != _material_data._vector_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealTensorValue> &
MaterialPropertyInterface::getTensorMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _material_data._tensor_props_old.find(name);

  if(it != _material_data._tensor_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<ColumnMajorMatrix> &
MaterialPropertyInterface::getColumnMajorMatrixMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _material_data._column_major_matrix_props_old.find(name);

  if(it != _material_data._column_major_matrix_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialPropertyInterface::getMatrixMaterialPropertyOld(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _material_data._matrix_props_old.find(name);

  if(it != _material_data._matrix_props_old.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}



Real &
MaterialPropertyInterface::getConstantRealMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, Real >::iterator it = _material_data._constant_real_props_older.find(name);

  if(it != _material_data._constant_real_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<Real> &
MaterialPropertyInterface::getRealMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<Real> >::iterator it = _material_data._real_props_older.find(name);

  if(it != _material_data._real_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealGradient> &
MaterialPropertyInterface::getGradientMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealGradient> >::iterator it = _material_data._gradient_props_older.find(name);

  if(it != _material_data._gradient_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealVectorValue> &
MaterialPropertyInterface::getRealVectorValueMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealVectorValue> >::iterator it = _material_data._real_vector_value_props_older.find(name);
  
  if(it != _material_data._real_vector_value_props_older.end())
    return it->second;
  
  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<Real> > &
MaterialPropertyInterface::getVectorMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<Real> > >::iterator it = _material_data._vector_props_older.find(name);

  if(it != _material_data._vector_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<RealTensorValue> &
MaterialPropertyInterface::getTensorMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<RealTensorValue> >::iterator it = _material_data._tensor_props_older.find(name);

  if(it != _material_data._tensor_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<ColumnMajorMatrix> &
MaterialPropertyInterface::getColumnMajorMatrixMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<ColumnMajorMatrix> >::iterator it = _material_data._column_major_matrix_props_older.find(name);

  if(it != _material_data._column_major_matrix_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}

MooseArray<MooseArray<MooseArray<Real> > > &
MaterialPropertyInterface::getMatrixMaterialPropertyOlder(const std::string & name)
{
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >::iterator it = _material_data._matrix_props_older.find(name);

  if(it != _material_data._matrix_props_older.end())
    return it->second;

  mooseError("Material has no property named: " + name);
}
