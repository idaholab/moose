// Original class author: A.M. Jokisaari, O. Heinonen

#include "FiniteStrainMaterial.h"

/**
 * FiniteStrainMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<FiniteStrainMaterial>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();

  return params;
}

FiniteStrainMaterial::FiniteStrainMaterial(const std::string & name, 
                                             InputParameters parameters)
    : TensorMechanicsMaterial(name, parameters)
{
}

void FiniteStrainMaterial::computeQpStrain()
{
  //Deformation gradient
  RankTwoTensor A(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]); //Deformation gradient
  RankTwoTensor Fbar(_grad_disp_x_old[_qp],_grad_disp_y_old[_qp],_grad_disp_z_old[_qp]); //Deformation gradient

  A -= Fbar;

  Fbar.addIa(1.0);
  
  //Incremental deformation gradient
  RankTwoTensor Fhat = A*Fbar.inverse();
  Fhat.addIa(1.0);

  //From Rashid, 1993
  RankTwoTensor U(Fhat.inverse());

  Real Ax = U(1,2) - U(2,1);
  Real Ay = U(2,0) - U(0,2);
  Real Az = U(0,1) - U(1,0);
  Real Q = (Ax*Ax + Ay*Ay + Az*Az)/4.0;
  Real P = (U.trace() - 1.0)*(U.trace() - 1.0);
  Real Y = 1.0/((Q + P)*(Q + P)*(Q + P));

  Real C1 = std::sqrt(P * (1 + (P*(Q+Q+(Q+P))) * (1-(Q+P)) * Y));
  Real C2 = 0.125 + Q * 0.03125 * (P*P - 12*(P-1)) / (P*P);
  Real C3 = 0.5 * std::sqrt( (P*Q*(3-Q) + P*P*P + Q*Q) / (P+Q)*(P+Q)*(P+Q) );

  RankTwoTensor R_incr;
  R_incr(0,0) = C1 + (C2*Ax)*Ax;
  R_incr(0,1) =      (C2*Ay)*Ax + (C3*Az);
  R_incr(0,2) =      (C2*Az)*Ax - (C3*Ay);
  R_incr(1,0) =      (C2*Ax)*Ay - (C3*Az);
  R_incr(1,1) = C1 + (C2*Ay)*Ay;
  R_incr(1,2) =      (C2*Az)*Ay + (C3*Ax);
  R_incr(2,0) =      (C2*Ax)*Az + (C3*Ay);
  R_incr(2,1) =      (C2*Ay)*Az - (C3*Ax);
  R_incr(2,2) = C1 + (C2*Az)*Az;
}

void FiniteStrainMaterial::computeQpStress()
{
  // stress = C * e
  _stress[_qp] = _elasticity_tensor[_qp]*_elastic_strain[_qp];
}
