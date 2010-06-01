#ifndef MATERIALPROPERTYINTERFACE_H
#define MATERIALPROPERTYINTERFACE_H

#include "MaterialData.h"

// Forward Declarations
class MaterialData;

class MaterialPropertyInterface
{
public:
  MaterialPropertyInterface(MaterialData & material_data);

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealMaterialProperty(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealMaterialProperty(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientMaterialProperty(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValueMaterialProperty(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorMaterialProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorMaterialProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixMaterialProperty(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixMaterialProperty(const std::string & name);

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealMaterialPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealMaterialPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientMaterialPropertyOld(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValueMaterialPropertyOld(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorMaterialPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorMaterialPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixMaterialPropertyOld(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixMaterialPropertyOld(const std::string & name);

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealMaterialPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealMaterialPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientMaterialPropertyOlder(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValueMaterialPropertyOlder(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorMaterialPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorMaterialPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixMaterialPropertyOlder(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixMaterialPropertyOlder(const std::string & name);

private:
  MaterialData & _material_data;
};

#endif //MATERIALPROPERTYINTERFACE_H
