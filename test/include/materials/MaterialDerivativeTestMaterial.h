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
#ifndef MATERIALDERIVATIVETESTMATERIAL_H_
#define MATERIALDERIVATIVETESTMATERIAL_H_

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class MaterialDerivativeTestMaterial;

template <>
InputParameters validParams<MaterialDerivativeTestMaterial>();

/**
 * A material used for testing the material derivative test kernel
 */
class MaterialDerivativeTestMaterial : public DerivativeMaterialInterface<Material>
{
public:
  MaterialDerivativeTestMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// the material property for which derivatives are to be tested
  MaterialProperty<Real> & _p;
  /// derivative of material property with respect to nonlinear variable 1
  MaterialProperty<Real> & _dpdu;
  /// derivative of material property with respect to nonlinear variable 2
  MaterialProperty<Real> & _dpdv;
  /// nonlinear variable 1
  const VariableValue & _u;
  /// nonlinear variable 2
  const VariableValue & _v;
};

#endif // MATERIALDERIVATIVETESTMATERIAL_H
