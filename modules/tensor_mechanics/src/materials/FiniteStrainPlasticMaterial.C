#include "FiniteStrainPlasticMaterial.h"

/**
FiniteStrainPlasticMaterial integrates the rate independent J2 plasticity model in a
finite strain framework using return mapping algorithm.
Integration is performed in an incremental manner using Newton Raphson.
Isotropic hardening has been incorporated where yield stress as a function of equivalent
plastic strain has to be specified by the user.
*/

template<>
InputParameters validParams<FiniteStrainPlasticMaterial>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRequiredParam< std::vector<Real> >("yield_stress", "Input data as pairs of equivalent plastic strain and yield stress: Should start with equivalent plastic strain 0");

  return params;
}

FiniteStrainPlasticMaterial::FiniteStrainPlasticMaterial(const std::string & name, 
                                             InputParameters parameters)
    : FiniteStrainMaterial(name, parameters),
      _yield_stress_vector(getParam< std::vector<Real> >("yield_stress")),//Read from input file
      _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
      _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
      _eqv_plastic_strain(declareProperty<Real>("eqv_plastic_strain"))

{
}

void FiniteStrainPlasticMaterial::initQpStatefulProperties()
{
  _elastic_strain[_qp].zero();
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
  _eqv_plastic_strain[_qp]=0.0;

  
}

void FiniteStrainPlasticMaterial::computeQpStress()
{

  RankTwoTensor dp,sig;

  
  //In elastic problem, all the strain is elastic
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];
  
  //Obtain previous plastic rate of deformation tensor
  dp=_plastic_strain_old[_qp];

  //Solve J2 plastic constitutive equations based on current strain increment
  //Returns current  stress and plastic rate of deformation tensor
  
  solveStressResid(_stress_old[_qp],_strain_increment[_qp],_elasticity_tensor[_qp],&dp,&sig);
  _stress[_qp]=sig;
  
  //Updates current plastic rate of deformation tensor
  _plastic_strain[_qp]=dp;

  //Evaluate and update current equivalent plastic strain
  _eqv_plastic_strain[_qp]=pow(2.0/3.0,0.5)*dp.L2norm();
  
  
  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();

  //Rotate to plastic rate of deformation tensor the current configuration
  _plastic_strain[_qp]=_rotation_increment[_qp]*_plastic_strain[_qp]*_rotation_increment[_qp].transpose();


}


/*
 *Solves for incremental plastic rate of deformation tensor and stress in unrotated frame.
 *Input: Strain incrment, 4th order elasticity tensor, stress tensor in previous incrmenent and
 *plastic rate of deformation tensor gradient. 
 */ 
void
FiniteStrainPlasticMaterial::solveStressResid(RankTwoTensor sig_old,RankTwoTensor delta_d,RankFourTensor E_ijkl, RankTwoTensor *dp, RankTwoTensor *sig)
{

  RankTwoTensor sig_new,delta_dp,dpn;
  RankTwoTensor flow_tensor, flow_dirn;
  RankTwoTensor resid,ddsig;
  RankFourTensor dr_dsig,dr_dsig_inv;
  Real /*sig_eqv,*/flow_incr,f,dflow_incr;
  Real err1,err2,err3,tol1,tol2,tol3;
  unsigned int plastic_flag;
  unsigned int iterisohard,iter,maxiterisohard=20,maxiter=20;
  Real flow_incr_tmp;
  Real eqvpstrain;
  Real yield_stress,yield_stress_prev;
  
  
  
  
  tol1=1e-10;
  tol2=1e-10;
  tol3=1e-6;
  
  sig_new=sig_old+E_ijkl*delta_d;
  eqvpstrain=pow(2.0/3.0,0.5)*dp->L2norm();
  yield_stress=getYieldStress(eqvpstrain);
  plastic_flag=isPlastic(sig_new,yield_stress);//Check of plasticity for elastic predictor
  
  
  if(plastic_flag==1)
  {

    iterisohard=0;
    eqvpstrain=pow(2.0/3.0,0.5)*dp->L2norm();
    yield_stress=getYieldStress(eqvpstrain);

    err3=1.1*tol3;
        
    while(err3 > tol3 && iterisohard < maxiterisohard)//Hardness update iteration
    {

      iterisohard++;


      iter=0;
      
      dflow_incr=0.0;
      flow_incr=0.0;
      delta_dp.zero();
      sig_new=sig_old+E_ijkl*delta_d;
            
      getFlowTensor(sig_new,&flow_tensor);
      flow_dirn=flow_tensor;
    

      resid=flow_dirn*flow_incr-delta_dp;//Residual 1 - refer Hughes Simo
      f=getSigEqv(sig_new)-yield_stress;//Residual 2 - f=0

      err1=resid.L2norm();
      err2=fabs(f);
    

      while((err1 > tol1 || err2 > tol2) && iter < maxiter )//Stress update iteration (hardness fixed)
      {


        iter++;
        
        getJac(sig_new,E_ijkl,flow_incr,&dr_dsig);//Jacobian
        dr_dsig_inv=dr_dsig.invSymm();
      
        ddsig=dr_dsig_inv*(-resid-flow_dirn*dflow_incr);
        dflow_incr=(f-flow_tensor.doubleContraction(dr_dsig_inv*resid))/(flow_tensor.doubleContraction(dr_dsig_inv*flow_dirn));

        flow_incr_tmp=flow_incr;
        flow_incr_tmp+=dflow_incr;
        if(flow_incr_tmp < 0.0)//negative flow increment not allowed
          dflow_incr=0.0;
      
        flow_incr+=dflow_incr;
        delta_dp-=E_ijkl.invSymm()*ddsig;
        sig_new+=ddsig;

        getFlowTensor(sig_new,&flow_tensor);
        flow_dirn=flow_tensor;

        resid=flow_dirn*flow_incr-delta_dp;
        f=getSigEqv(sig_new)-yield_stress;
      
        err1=resid.L2norm();
        err2=fabs(f);
      }

      if(iter>=maxiter)
        printf("Constitutive Error: Too many iterations\n");//Convergence failure
      
      dpn=*dp+delta_dp;
      eqvpstrain=pow(2.0/3.0,0.5)*dpn.L2norm();

      yield_stress_prev=yield_stress;
      yield_stress=getYieldStress(eqvpstrain);

      err3=fabs(yield_stress-yield_stress_prev);
      
    }

    if(iterisohard>=maxiterisohard)
      printf("Constitutive Error: Too many iterations in Hardness Update\n");//Convergence failure
    
      
  }

  *dp=dpn;//Plastic rate of deformation tensor in unrotated configuration

  *sig=sig_new;
}



//Check for yield condition
unsigned int
FiniteStrainPlasticMaterial::isPlastic(RankTwoTensor sig,Real yield_stress)
{
  Real sig_eqv;
  Real toly=1e-8;
  
  sig_eqv=getSigEqv(sig);
  
  if(sig_eqv-yield_stress>toly)
  {
    return 1; 
  }
  else
  {
    return 0;
  }
}



//Obtain derivative of yield surface w.r.t. stress (plastic flow direction)
void
FiniteStrainPlasticMaterial::getFlowTensor(RankTwoTensor sig,RankTwoTensor* flow_tensor)
{

  RankTwoTensor sig_dev;
  Real sig_eqv,val;
  
  sig_eqv=getSigEqv(sig);
  sig_dev=getSigDev(sig);

  val=3.0/(2.0*sig_eqv);
  *flow_tensor=sig_dev*val;  
}

//Obtain equivalent stress (scalar)
Real
FiniteStrainPlasticMaterial::getSigEqv(RankTwoTensor sig)
{

  Real sig_eqv,sij;
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

//Obtain deviatoric stress tensor
RankTwoTensor
FiniteStrainPlasticMaterial::getSigDev(RankTwoTensor sig)
{

//  Real sig_eqv,sij;
  RankTwoTensor identity;
  RankTwoTensor sig_dev;

  for(int i=0; i<3; i++)
    identity(i,i)=1.0;
  
  sig_dev=sig-identity*sig.trace()/3.0;
  return sig_dev;
  
  
}

//Jacobian for stress update algorithm
void
FiniteStrainPlasticMaterial::getJac(RankTwoTensor sig, RankFourTensor E_ijkl, Real flow_incr, RankFourTensor* dresid_dsig)
{

  RankTwoTensor sig_dev, flow_tensor, flow_dirn;
  RankTwoTensor dfi_dft,dfi_dsig;
  RankFourTensor dft_dsig,dfd_dft,dfd_dsig;
  Real sig_eqv/*,val*/;
  Real f1,f2,f3;
  RankFourTensor temp;
  
  
  sig_dev=getSigDev(sig);
  sig_eqv=getSigEqv(sig);
  getFlowTensor(sig,&flow_tensor);
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
  *dresid_dsig=E_ijkl.invSymm()+dfd_dsig*flow_incr;
  
  
}



//Delta Function
Real
FiniteStrainPlasticMaterial::deltaFunc(int i, int j)
{
  if(i==j)
    return 1.0;
  else
    return 0.0;
}


//Obtain yield stress for a given equivalent plastic strain (input)
Real
FiniteStrainPlasticMaterial::getYieldStress(Real eqpe)
{

  int nsize;
  Real *data;
  
  nsize=_yield_stress_vector.size();
  data=_yield_stress_vector.data();

  if(data[0] > 0.0 || nsize%2 >0 )//Error check for input inconsitency
  {
    printf("Error in yield stress input: Should be a vector with eqv plastic strain and yield stress pair values.\n");

  }

  int ind=0;
  Real tol=1e-8;
  
  
  while(ind<nsize)
  {
    
    if(fabs(eqpe-data[ind])<tol)
      return data[ind+1];

    if(ind+2<nsize)
    {
      if(eqpe > data[ind] && eqpe < data[ind+2])
      {
        return data[ind+1]+(eqpe-data[ind])/(data[ind+2]-data[ind])*(data[ind+3]-data[ind+1]);
        
      }
      
    }
    else
    {
      return data[nsize-1];     
      
    }

    ind+=2;;
  }
  

  return 0.0;
  

}
