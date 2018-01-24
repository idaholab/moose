/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
