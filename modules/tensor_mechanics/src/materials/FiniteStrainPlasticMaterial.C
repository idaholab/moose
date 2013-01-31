// Original class author: A.M. Jokisaari, O. Heinonen

#include "FiniteStrainPlasticMaterial.h"

/**
 * FiniteStrainElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<FiniteStrainPlasticMaterial>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRequiredParam<Real>("yield_stress", "Yield stress for perfectly plastic material");

  return params;
}

FiniteStrainPlasticMaterial::FiniteStrainPlasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : FiniteStrainMaterial(name, parameters),
      _yield_stress(parameters.get<Real>("yield_stress")),
      _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
      _plastic_strain_old(declareProperty<RankTwoTensor>("plastic_strain"))
{
}


void FiniteStrainPlasticMaterial::initQpStatefulProperties()
{
  _plastic_strain[_qp].zero();

}

void FiniteStrainPlasticMaterial::computeQpStress()
{

  RankTwoTensor sig_old, sig_new, delta_d;
  RankFourTensor E_ijkl;
  
  
  //In elastic problem, all the strain is elastic
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];
  _plastic_strain[_qp]=_plastic_strain_old[_qp];
  

  
  sig_old=_stress_old[_qp];
  delta_d=_strain_increment[_qp];
  E_ijkl=_elasticity_tensor[_qp];
  

  sig_new=solveStressResid(sig_old,delta_d,E_ijkl);
  _stress[_qp] =sig_new;
  
  
  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();


}


RankTwoTensor
FiniteStrainPlasticMaterial::solveStressResid(RankTwoTensor sig_old,RankTwoTensor delta_d,RankFourTensor E_ijkl)
{

  RankTwoTensor sig_new,delta_dp;
  RankTwoTensor flow_tensor, flow_dirn;
  RankTwoTensor resid,ddsig;
  RankFourTensor dr_dsig,dr_dsig_inv;
  Real sig_eqv,flow_incr,f,dflow_incr;
  Real err1,err2,tol;
  unsigned int plastic_flag;
  unsigned int maxiter=20;
  int iter=0;
  Real flow_incr_tmp;
  
  
//  printf("Mat Point\n");

  tol=1e-6;

  dflow_incr=0.0;
  flow_incr=0.0;
  sig_new=sig_old+E_ijkl*delta_d;  
  plastic_flag=isPlastic(sig_new);
  
  if(plastic_flag==1)
  {

    flow_tensor=getFlowTensor(sig_new);
    flow_dirn=flow_tensor;
    

    resid=flow_dirn*flow_incr-delta_dp;
    f=getSigEqv(sig_new)-_yield_stress;

    err1=resid.L2norm();
    err2=fabs(f);
    

    while((err1 > tol*_yield_stress || err2 > tol*_yield_stress) && iter < maxiter )
    {


      iter=iter+1;
      
      dr_dsig=getJac(sig_new,E_ijkl,flow_incr);
      dr_dsig_inv=dr_dsig.invSymm();
      
      ddsig=dr_dsig_inv*(-resid-flow_dirn*dflow_incr);
      dflow_incr=(f-flow_tensor.doubleContraction(dr_dsig_inv*resid))/(flow_tensor.doubleContraction(dr_dsig_inv*flow_dirn));

      flow_incr_tmp=flow_incr;
      flow_incr_tmp+=dflow_incr;
      if(flow_incr_tmp < 0.0)
        dflow_incr=0.0;
      

      
      flow_incr+=dflow_incr;
      delta_dp-=E_ijkl.invSymm()*ddsig;
      sig_new+=ddsig;

      flow_tensor=getFlowTensor(sig_new);
      flow_dirn=flow_tensor;

      resid=flow_dirn*flow_incr-delta_dp;
      f=getSigEqv(sig_new)-_yield_stress;
      
      err1=resid.L2norm();
      err2=fabs(f);
    }      
  }

  


  if(iter>=maxiter)
    printf("Constitutive Error: Too many iterations\n");
  
  
  return sig_new;
}




unsigned int
FiniteStrainPlasticMaterial::isPlastic(RankTwoTensor sig)
{
  Real sig_eqv;
  Real toly=1e-6;
  
  sig_eqv=getSigEqv(sig);
  
  if(sig_eqv-_yield_stress>toly*_yield_stress)
  {
    return 1; 
  }
  else
  {
    return 0;
  }
}



RankTwoTensor
FiniteStrainPlasticMaterial::getFlowTensor(RankTwoTensor sig)
{

  RankTwoTensor flow_tensor,sig_dev;
  Real sig_eqv,val;
  
  sig_eqv=getSigEqv(sig);
  sig_dev=getSigDev(sig);

  val=3.0/(2.0*sig_eqv);
  flow_tensor=sig_dev*val;
  
  
  return flow_tensor;
  
}

Real
FiniteStrainPlasticMaterial::getSigEqv(RankTwoTensor sig)
{

  Real sig_eqv,sij;
  RankTwoTensor identity;
  RankTwoTensor sig_dev;
  
  sig_dev=getSigDev(sig);
  sig_eqv=0.0;
  
  
  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
    {
      sij=sig_dev(i,j);
      sig_eqv+=sij*sij;
    }
  
  sig_eqv=3.0*sig_eqv/2.0;
  sig_eqv=pow(sig_eqv,0.5);

  return sig_eqv;
}

RankTwoTensor
FiniteStrainPlasticMaterial::getSigDev(RankTwoTensor sig)
{

  Real sig_eqv,sij;
  RankTwoTensor identity;
  RankTwoTensor sig_dev;

  for(int i=0; i<3; i++)
    identity(i,i)=1.0;
  
  sig_dev=sig-identity*sig.trace()/3.0;
  return sig_dev;
  
  
}

RankFourTensor
FiniteStrainPlasticMaterial::getJac(RankTwoTensor sig, RankFourTensor E_ijkl, Real flow_incr)
{

  RankTwoTensor sig_dev, flow_tensor, flow_dirn;
  RankTwoTensor dfi_dft,dfi_dsig;
  RankFourTensor dft_dsig,dfd_dft,dfd_dsig;
  RankFourTensor dresid_dsig;
  Real sig_eqv,val;
  Real f1,f2,f3;
  RankFourTensor temp;
  
  
  sig_dev=getSigDev(sig);
  sig_eqv=getSigEqv(sig);
  flow_tensor=getFlowTensor(sig);
  flow_dirn=flow_tensor;


  f1=3.0/(2.0*sig_eqv);
  f2=f1/3.0;
  f3=9.0/(4.0*pow(sig_eqv,3.0)); 
  
  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++)
      for(int k=0;k<3;k++)
        for(int l=0;l<3;l++)
          dft_dsig(i,j,k,l)=f1*deltaFunc(i,k)*deltaFunc(j,l)-f2*deltaFunc(i,j)*deltaFunc(k,l)-f3*sig_dev(i,j)*sig_dev(k,l);
  
  dfd_dsig=dft_dsig;
  dresid_dsig=E_ijkl.invSymm()+dfd_dsig*flow_incr;  
  return dresid_dsig;
  
  
}




Real
FiniteStrainPlasticMaterial::deltaFunc(int i, int j)
{
  if(i==j)
    return 1.0;
  else
    return 0.0;
}


