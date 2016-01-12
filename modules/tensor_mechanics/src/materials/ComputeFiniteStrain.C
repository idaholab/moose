/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeFiniteStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ComputeFiniteStrain>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains.");
  params.set<bool>("stateful_displacements") = true;
  return params;
}

ComputeFiniteStrain::ComputeFiniteStrain(const InputParameters & parameters) :
    ComputeStrainBase(parameters),
    _strain_rate(declareProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _mechanical_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _deformation_gradient(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _deformation_gradient_old(declarePropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _stress_free_strain_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "stress_free_strain_increment")),
    _T_old(coupledValueOld("temperature"))
{
}

void
ComputeFiniteStrain::initQpStatefulProperties()
{
  ComputeStrainBase::initQpStatefulProperties();

  _strain_rate[_qp].zero();
  _strain_increment[_qp].zero();
  _rotation_increment[_qp].zero();
  _deformation_gradient[_qp].zero();
  _deformation_gradient[_qp].addIa(1.0);
  _deformation_gradient_old[_qp] = _deformation_gradient[_qp];
  _mechanical_strain_old[_qp] = _mechanical_strain[_qp];
  _total_strain_old[_qp] = _total_strain[_qp];
}

void
ComputeFiniteStrain::computeProperties()
{
  //Method from Rashid, 1993
  std::vector<RankTwoTensor> Fhat;
  Fhat.resize(_qrule->n_points());
  RankTwoTensor ave_Fhat;
  Real volume(0);
  Real ave_dfgrd_det;

  ave_dfgrd_det=0.0;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    //Deformation gradient
    RankTwoTensor A((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); //Deformation gradient
    RankTwoTensor Fbar((*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]); //Old Deformation gradient

    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);//Gauss point deformation gradient

    A -= Fbar; //A = gradU - gradUold

    Fbar.addIa(1.0); //Fbar = ( I + gradUold)

    //Incremental deformation gradient Fhat = I + A Fbar^-1
    Fhat[_qp] = A * Fbar.inverse();
    Fhat[_qp].addIa(1.0);

    //Calculate average Fhat for volumetric locking correction
    ave_Fhat += Fhat[_qp] * _JxW[_qp];
    volume += _JxW[_qp];

    ave_dfgrd_det += _deformation_gradient[_qp].det() * _JxW[_qp]; //Average deformation gradient
  }

  ave_Fhat /= volume; //This is needed for volumetric locking correction
  ave_dfgrd_det /=volume; //Average deformation gradient

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real factor( std::pow( ave_Fhat.det() / Fhat[_qp].det(), 1.0/3.0));
    Fhat[_qp] *= factor; //Finalize volumetric locking correction

    computeQpStrain(Fhat[_qp]);

    factor = std::pow(ave_dfgrd_det / _deformation_gradient[_qp].det(), 1.0/3.0);//Volumetric locking correction
    _deformation_gradient[_qp] *= factor;//Volumetric locking correction
  }
}

void
ComputeFiniteStrain::computeQpStrain(const RankTwoTensor & Fhat)
{
  //Cinv - I = A A^T - A - A^T;
  RankTwoTensor A; //A = I - Fhatinv
  A.addIa(1.0);
  A -= Fhat.inverse();
  RankTwoTensor Cinv_I = A*A.transpose() - A - A.transpose();

  //strain rate D from Taylor expansion, Chat = (-1/2(Chat^-1 - I) + 1/4*(Chat^-1 - I)^2 + ...
  RankTwoTensor total_strain_increment = -Cinv_I*0.5 + Cinv_I*Cinv_I*0.25;

  _strain_increment[_qp] = total_strain_increment;

  //Remove thermal expansion
  _strain_increment[_qp].addIa(-_thermal_expansion_coeff*( _T[_qp] - _T_old[_qp]));

  //Remove the Eigen strain increment
  _strain_increment[_qp] -= _stress_free_strain_increment[_qp];

  RankTwoTensor D = _strain_increment[_qp]/_dt;
  _strain_rate[_qp] = D;

  //Calculate rotation R_incr
  RankTwoTensor invFhat(Fhat.inverse());

  std::vector<Real> a(3);
  a[0] = invFhat(1,2) - invFhat(2,1);
  a[1] = invFhat(2,0) - invFhat(0,2);
  a[2] = invFhat(0,1) - invFhat(1,0);
  Real q = (a[0]*a[0] + a[1]*a[1] + a[2]*a[2])/4.0;
  Real trFhatinv_1 = invFhat.trace() - 1.0;
  Real p = trFhatinv_1*trFhatinv_1/4.0;

  Real C1 = std::sqrt(p + 3.0*p*p*(1.0 - (p + q))/((p+q)*(p+q)) - 2.0*p*p*p*(1-(p+q))/((p+q)*(p+q)*(p+q))); //cos theta_a
  Real C2 = 0.0;
  if (q > 0.01)
    C2 = (1.0 - C1)/(4.0*q); // (1-cos theta_a)/4q
  else //alternate form for small q
    C2 = 0.125 + q*0.03125*(p*p - 12*(p-1))/(p*p) + q*q*(p - 2.0)*(p*p - 10.0*p + 32.0)/(p*p*p) + q*q*q*(1104.0 - 992.0*p + 376.0*p*p - 72*p*p*p + 5.0*p*p*p*p)/(512.0*p*p*p*p);

  Real C3 = 0.5*std::sqrt((p*q*(3.0 - q) + p*p*p + q*q)/((p + q)*(p + q)*(p + q))); //sin theta_a/(2 sqrt(q))

  //Calculate incremental rotation. Note that this value is the transpose of that from Rashid, 93, so we transpose it before storing
  RankTwoTensor R_incr;
  R_incr.addIa(C1);
  for (unsigned int i=0; i<3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      R_incr(i,j) += C2*a[i]*a[j];

  R_incr(0,1) += C3*a[2];
  R_incr(0,2) -= C3*a[1];
  R_incr(1,0) -= C3*a[2];
  R_incr(1,2) += C3*a[0];
  R_incr(2,0) += C3*a[1];
  R_incr(2,1) -= C3*a[0];
  _rotation_increment[_qp] = R_incr.transpose();

  //Update strain in intermediate configuration
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;

  //Rotate strain to current configuration
  _mechanical_strain[_qp] = _rotation_increment[_qp] * _mechanical_strain[_qp] * _rotation_increment[_qp].transpose();
  _total_strain[_qp] = _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();
}
