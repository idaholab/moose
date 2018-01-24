//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TYPESMATERIAL_H
#define TYPESMATERIAL_H

#include "Material.h"
// libMesh
#include "libmesh/dense_matrix.h"

// Forward Declarations
class TypesMaterial;

template <>
InputParameters validParams<TypesMaterial>();

/**
 * Material for testing different types of material properties
 */
class TypesMaterial : public Material
{
public:
  TypesMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialProperty<Real> & _real_prop;
  MaterialProperty<std::vector<Real>> & _std_vec_prop;
  MaterialProperty<std::vector<Real>> & _std_vec_prop_qp;
  MaterialProperty<std::vector<RealGradient>> & _std_vec_grad_prop;
  MaterialProperty<RealVectorValue> & _real_vec_prop;
  MaterialProperty<DenseMatrix<Real>> & _matrix_prop;
  MaterialProperty<RealTensorValue> & _tensor_prop;
};

#endif // TYPESMATERIAL_H
