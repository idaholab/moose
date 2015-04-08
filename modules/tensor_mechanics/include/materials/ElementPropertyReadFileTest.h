/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELEMENTPROPERTYREADFILETEST_H
#define ELEMENTPROPERTYREADFILETEST_H

/**
 * Test userobject ElementPropertyReadFile
 */

#include "FiniteStrainElasticMaterial.h"
#include "ElementPropertyReadFile.h"

class ElementPropertyReadFileTest;

template<>
InputParameters validParams<ElementPropertyReadFileTest>();

class ElementPropertyReadFileTest : public FiniteStrainElasticMaterial
{
public:
  ElementPropertyReadFileTest(const std::string & name, InputParameters parameters);

protected:
  /**
   * This function calls UserObject to assign properties read from file to stateful properties
   */
  virtual void initQpStatefulProperties();

  /**
   * This function reads euler_angle read from the UserObject
   * and compute rotated elasticity tensor
   */
  virtual void computeQpElasticityTensor();

  /**
   * This function compute rotation tensor from euler_angle
   */
  void getEulerRotations();

  /// UserObject variable
  const ElementPropertyReadFile * _read_prop_user_object;
  /// State variable
  MaterialProperty<Real> & _some_state_var;
  MaterialProperty<Real> & _some_state_var_old;

  RankTwoTensor _crysrot;

};

#endif //ELEMENTPROPERTYREADFILETEST_H
