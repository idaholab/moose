#ifndef TYPESMATERIAL_H
#define TYPESMATERIAL_H

#include "Material.h"
// libMesh
#include "libmesh/dense_matrix.h"

//Forward Declarations
class TypesMaterial;

template<>
InputParameters validParams<TypesMaterial>();

/**
 * Material for testing different types of material properties
 */
class TypesMaterial : public Material
{
public:
  TypesMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _real_prop;
  MaterialProperty<std::vector<Real> > & _std_vec_prop;
  MaterialProperty<RealVectorValue> & _real_vec_prop;
  MaterialProperty<DenseMatrix<Real> > & _matrix_prop;
  MaterialProperty<RealTensorValue> & _tensor_prop;
};

#endif //TYPESMATERIAL_H
