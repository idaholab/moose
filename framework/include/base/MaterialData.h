#ifndef MATERIALDATA_H
#define MATERIALDATA_H

//MOOSE includes
#include "Moose.h"
#include "MooseArray.h"

//libMesh includes
#include "transient_system.h"

//Forward Declarations
class MooseSystem;
class QGauss;
class DofMap;
class FEBase;
class ColumnMajorMatrix;
template<class T> class NumericVector;
template<class T> class DenseVector;
template<class T> class DenseSubVector;
template<class T> class DenseMatrix;

class MaterialData
{
public:
  MaterialData(MooseSystem & moose_system);

  MooseSystem & _moose_system;

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealProperty(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealProperty(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientProperty(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValueProperty(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixProperty(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixProperty(const std::string & name);

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientPropertyOld(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValuePropertyOld(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixPropertyOld(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixPropertyOld(const std::string & name);

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientPropertyOlder(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValuePropertyOlder(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixPropertyOlder(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixPropertyOlder(const std::string & name);


  // These are what the Material objects actually get references to
  std::map<std::string, Real >                                        _constant_real_props;
  std::map<std::string, MooseArray<Real> >                            _real_props;
  std::map<std::string, MooseArray<RealGradient> >                    _gradient_props;
  std::map<std::string, MooseArray<RealVectorValue> >                 _real_vector_value_props;
  std::map<std::string, MooseArray<MooseArray<Real> > >               _vector_props;
  std::map<std::string, MooseArray<RealTensorValue> >                 _tensor_props;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >               _column_major_matrix_props;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >  _matrix_props;

  std::map<std::string, Real >                                        _constant_real_props_old;
  std::map<std::string, MooseArray<Real> >                            _real_props_old;
  std::map<std::string, MooseArray<RealGradient> >                    _gradient_props_old;
  std::map<std::string, MooseArray<RealVectorValue> >                 _real_vector_value_props_old;
  std::map<std::string, MooseArray<MooseArray<Real> > >               _vector_props_old;
  std::map<std::string, MooseArray<RealTensorValue> >                 _tensor_props_old;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >               _column_major_matrix_props_old;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >  _matrix_props_old;

  std::map<std::string, Real >                                        _constant_real_props_older;
  std::map<std::string, MooseArray<Real> >                            _real_props_older;
  std::map<std::string, MooseArray<RealGradient> >                    _gradient_props_older;
  std::map<std::string, MooseArray<RealVectorValue> >                 _real_vector_value_props_older;
  std::map<std::string, MooseArray<MooseArray<Real> > >               _vector_props_older;
  std::map<std::string, MooseArray<RealTensorValue> >                 _tensor_props_older;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >               _column_major_matrix_props_older;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > >  _matrix_props_older;

};
#endif //MATERIALDATA_H
