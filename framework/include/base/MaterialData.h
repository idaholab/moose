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
