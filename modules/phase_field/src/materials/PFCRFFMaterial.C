#include "PFCRFFMaterial.h"

template<>
InputParameters validParams<PFCRFFMaterial>()
{
  InputParameters params = validParams<Material>();
  
  return params;
}

PFCRFFMaterial::PFCRFFMaterial(const std::string & name,
                 InputParameters parameters)
  :Material(name, parameters),
   _M(declareProperty<Real>("M")),
   _alpha_R_0(declareProperty<Real>("alpha_R_0")),
   _alpha_I_0(declareProperty<Real>("alpha_I_0")),
   _A_R_0(declareProperty<Real>("A_R_0")),
   _A_I_0(declareProperty<Real>("A_I_0")),
   _alpha_R_1(declareProperty<Real>("alpha_R_1")),
   _alpha_I_1(declareProperty<Real>("alpha_I_1")),
   _A_R_1(declareProperty<Real>("A_R_1")),
   _A_I_1(declareProperty<Real>("A_I_1")),
   _alpha_R_2(declareProperty<Real>("alpha_R_2")),
   _alpha_I_2(declareProperty<Real>("alpha_I_2")),
   _A_R_2(declareProperty<Real>("A_R_2")),
   _A_I_2(declareProperty<Real>("A_I_2"))
{
}

void
PFCRFFMaterial::computeQpProperties()
{
  //Mobility
  _M[_qp] = 1.0;
  
  //alpha constants
  _alpha_R_0[_qp] = -2.429134088464706;
  _alpha_I_0[_qp] =  0.0;
  _alpha_R_1[_qp] = 18.943264072194637;
  _alpha_I_1[_qp] =  9.349446845430961;
  _alpha_R_2[_qp] =  3.972333899872749;
  _alpha_I_2[_qp] =  6.499130135847140;

  //A constants
  _A_R_0[_qp] = -1.282478656880326e2;
  _A_I_0[_qp] =  9.910190130869531e-15;
  _A_R_1[_qp] = 10.501019149026910;
  _A_I_1[_qp] =  2.363585467971611;
  _A_R_2[_qp] = 34.212475550666770;
  _A_I_2[_qp] = -42.274652746496530;
  
  
}

