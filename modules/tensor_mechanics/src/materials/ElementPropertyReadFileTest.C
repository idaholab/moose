/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElementPropertyReadFileTest.h"
#include <cmath>

template<>
InputParameters validParams<ElementPropertyReadFileTest>()
{
  InputParameters params = validParams<FiniteStrainElasticMaterial>();
  params.addClassDescription("Material class to test ElementPropertyReadFile User Object");
  params.addParam<UserObjectName>("read_prop_user_object","The ElementReadPropertyFile GeneralUserObject to read element specific property values from file");

  return params;
}

ElementPropertyReadFileTest::ElementPropertyReadFileTest(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainElasticMaterial(name, parameters),
    _read_prop_user_object(isParamValid("read_prop_user_object") ? & getUserObject<ElementPropertyReadFile>("read_prop_user_object") : NULL),
    _some_state_var(declareProperty<Real>("some_state_var")),
    _some_state_var_old(declarePropertyOld<Real>("some_state_var"))
{
}


void ElementPropertyReadFileTest::initQpStatefulProperties()
{
  _some_state_var[_qp] = _some_state_var_old[_qp] = 0.0;

  if ( _read_prop_user_object )
    //Stateful properties read from file used in initialization
    _some_state_var[_qp] = _some_state_var_old[_qp] = _read_prop_user_object->getData( _current_elem , 3 );
}


void
ElementPropertyReadFileTest::computeQpElasticityTensor()
{
  //Properties assigned at the beginning of every call to material calculation
  if ( _read_prop_user_object )
  {
    _Euler_angles(0) = _read_prop_user_object->getData( _current_elem , 0 );
    _Euler_angles(1) = _read_prop_user_object->getData( _current_elem , 1 );
    _Euler_angles(2) = _read_prop_user_object->getData( _current_elem , 2 );
  }

  getEulerRotations();

  RealTensorValue rot;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot(i,j) = _crysrot(i,j);

  _elasticity_tensor[_qp] = _Cijkl;
  _elasticity_tensor[_qp].rotate(rot);

  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

void
ElementPropertyReadFileTest::getEulerRotations()
{
  Real phi1, phi, phi2;
  Real cp, cp1, cp2, sp, sp1, sp2;
  RankTwoTensor RT;
  Real pi = libMesh::pi;

  phi1 = _Euler_angles(0) * (pi/180.0);
  phi =  _Euler_angles(1) * (pi/180.0);
  phi2 = _Euler_angles(2) * (pi/180.0);

  cp1 = std::cos(phi1);
  cp2 = std::cos(phi2);
  cp = std::cos(phi);

  sp1 = std::sin(phi1);
  sp2 = std::sin(phi2);
  sp = std::sin(phi);

  RT(0,0) = cp1 * cp2 - sp1 * sp2 * cp;
  RT(0,1) = sp1 * cp2 + cp1 * sp2 * cp;
  RT(0,2) = sp2 * sp;
  RT(1,0) = -cp1 * sp2 - sp1 * cp2 * cp;
  RT(1,1) = -sp1 * sp2 + cp1 * cp2 * cp;
  RT(1,2) = cp2 * sp;
  RT(2,0) = sp1 * sp;
  RT(2,1) = -cp1 * sp;
  RT(2,2) = cp;

  _crysrot = RT.transpose();
}
