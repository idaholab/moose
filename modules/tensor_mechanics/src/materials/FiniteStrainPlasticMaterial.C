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
  params.addParam<Real>("rtol",1e-8,"Plastic strain NR tolerance");
  params.addParam<Real>("ftol",1e-4,"Consistency condition NR tolerance");
  params.addParam<Real>("eptol",1e-7,"Equivalent plastic strain NR tolerance");

  return params;
}

FiniteStrainPlasticMaterial::FiniteStrainPlasticMaterial(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
    _yield_stress_vector(getParam< std::vector<Real> >("yield_stress")),//Read from input file
    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _eqv_plastic_strain(declareProperty<Real>("eqv_plastic_strain")),
    _eqv_plastic_strain_old(declarePropertyOld<Real>("eqv_plastic_strain")),
    _rtol(getParam<Real>("rtol")),
    _ftol(getParam<Real>("ftol")),
    _eptol(getParam<Real>("eptol"))
{
}

void FiniteStrainPlasticMaterial::initQpStatefulProperties()
{
  _elastic_strain[_qp].zero();
  _stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _plastic_strain_old[_qp].zero();
  _eqv_plastic_strain[_qp] = 0.0;
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

  solveStressResid(_stress_old[_qp], _strain_increment[_qp], _elasticity_tensor[_qp], dp, sig);
  _stress[_qp] = sig;

  //Updates current plastic rate of deformation tensor
  _plastic_strain[_qp] = dp;

  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();

  //Rotate to plastic rate of deformation tensor the current configuration
  _plastic_strain[_qp] = _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();
}


/*
 *Solves for incremental plastic rate of deformation tensor and stress in unrotated frame.
 *Input: Strain incrment, 4th order elasticity tensor, stress tensor in previous incrmenent and
 *plastic rate of deformation tensor gradient.
 */
void
FiniteStrainPlasticMaterial::solveStressResid(const RankTwoTensor & sig_old, const RankTwoTensor & delta_d, const RankFourTensor & E_ijkl, RankTwoTensor & dp, RankTwoTensor & sig)
{
  RankTwoTensor sig_new, delta_dp, dpn;
  RankTwoTensor flow_tensor, flow_dirn;
  RankTwoTensor resid, ddsig;
  RankFourTensor dr_dsig, dr_dsig_inv;
  Real flow_incr, f, dflow_incr;
  Real err1, err2, err3;
  unsigned int plastic_flag;
  unsigned int iter, maxiter = 100;
  Real eqvpstrain, eqvpstrain_old, deqvpstrain, fq, rep;
  Real yield_stress;

  sig_new = sig_old + E_ijkl * delta_d;
  eqvpstrain_old = eqvpstrain = _eqv_plastic_strain_old[_qp];
  yield_stress = getYieldStress(eqvpstrain);
  plastic_flag = isPlastic(sig_new, yield_stress); //Check of plasticity for elastic predictor

  if (plastic_flag == 1)
  {
    iter = 0;

    dflow_incr = 0.0;
    flow_incr = 0.0;
    delta_dp.zero();
    deqvpstrain = 0.0;

    sig_new = sig_old + E_ijkl * delta_d;

    getFlowTensor(sig_new, flow_tensor);
    flow_dirn = flow_tensor;

    resid = flow_dirn * flow_incr - delta_dp; //Residual 1 - refer Hughes Simo
    f = getSigEqv(sig_new) - yield_stress; //Residual 2 - f=0
    rep = -eqvpstrain+eqvpstrain_old + flow_incr; //Residual 3 rep=0

    err1 = resid.L2norm();
    err2 = std::abs(f);
    err3 = std::abs(rep);

    while ((err1 > _rtol || err2 > _ftol || err3 > _eptol) && iter < maxiter )//Stress update iteration (hardness fixed)
    {
      iter++;

      getJac(sig_new, E_ijkl, flow_incr, dr_dsig); //Jacobian
      dr_dsig_inv = dr_dsig.invSymm();
      fq=getdYieldStressdPlasticStrain(eqvpstrain);

      dflow_incr = (f - flow_tensor.doubleContraction(dr_dsig_inv * resid) + fq * rep) / (flow_tensor.doubleContraction(dr_dsig_inv * flow_dirn) - fq);
      ddsig = dr_dsig_inv * (-resid - flow_dirn * dflow_incr);

      flow_incr += dflow_incr;
      delta_dp -= E_ijkl.invSymm() * ddsig;
      sig_new += ddsig;
      deqvpstrain=rep+dflow_incr;
      eqvpstrain+=deqvpstrain;

      getFlowTensor(sig_new, flow_tensor);
      flow_dirn=flow_tensor;

      resid=flow_dirn*flow_incr-delta_dp;
      f=getSigEqv(sig_new)-yield_stress;
      rep=-eqvpstrain+eqvpstrain_old+flow_incr;

      err1 = resid.L2norm();
      err2 = std::abs(f);
      err3 = std::abs(rep);

    }

    if (iter>=maxiter)
    {
      _stress[_qp](2,2) = 1.0e6;
      _console << "Constitutive Error: Too many iterations " << err1 << ' ' <<  err2 << ' ' << err3 << '\n'; //Convergence failure
      return;
    }

    dpn = dp + delta_dp;
  }

  dp = dpn; //Plastic strain in unrotated configuration
  sig = sig_new;
  _eqv_plastic_strain[_qp] = eqvpstrain;
}

//Check for yield condition
unsigned int
FiniteStrainPlasticMaterial::isPlastic(const RankTwoTensor & sig, Real yield_stress)
{
  Real sig_eqv;
  Real toly = 1.0e-8;

  sig_eqv = getSigEqv(sig);

  if (sig_eqv-yield_stress>toly)
    return 1;
  else
    return 0;
}

//Obtain derivative of yield surface w.r.t. stress (plastic flow direction)
void
FiniteStrainPlasticMaterial::getFlowTensor(const RankTwoTensor & sig, RankTwoTensor & flow_tensor)
{
  RankTwoTensor sig_dev;
  Real sig_eqv, val;

  sig_eqv = getSigEqv(sig);
  sig_dev = getSigDev(sig);

  val = 3.0 / (2.0 * sig_eqv);
  flow_tensor = sig_dev * val;
}

//Obtain equivalent stress (scalar)
Real
FiniteStrainPlasticMaterial::getSigEqv(const RankTwoTensor & sig)
{
  Real sig_eqv, sij;
  RankTwoTensor sig_dev;

  sig_dev = getSigDev(sig);
  sig_eqv = 0.0;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
    {
      sij = sig_dev(i,j);
      sig_eqv += sij * sij;
    }

  sig_eqv = 3.0 * sig_eqv / 2.0;
  sig_eqv = std::pow(sig_eqv, 0.5);

  return sig_eqv;
}

//Obtain deviatoric stress tensor
RankTwoTensor
FiniteStrainPlasticMaterial::getSigDev(const RankTwoTensor & sig)
{
  RankTwoTensor identity;
  RankTwoTensor sig_dev;

  for (unsigned int i = 0; i < 3; ++i)
    identity(i,i) = 1.0;

  sig_dev = sig - identity * sig.trace() / 3.0;
  return sig_dev;
}

//Jacobian for stress update algorithm
void
FiniteStrainPlasticMaterial::getJac(const RankTwoTensor & sig, const RankFourTensor & E_ijkl, Real flow_incr, RankFourTensor & dresid_dsig)
{
  unsigned int i, j, k, l;
  RankTwoTensor sig_dev, flow_tensor, flow_dirn;
  RankTwoTensor dfi_dft, dfi_dsig;
  RankFourTensor dft_dsig, dfd_dft, dfd_dsig;
  Real sig_eqv;
  Real f1, f2, f3;
  RankFourTensor temp;

  sig_dev = getSigDev(sig);
  sig_eqv = getSigEqv(sig);
  getFlowTensor(sig, flow_tensor);
  flow_dirn = flow_tensor;


  f1 = 3.0 / (2.0 * sig_eqv);
  f2 = f1 / 3.0;
  f3 = 9.0 / (4.0 * std::pow(sig_eqv, 3.0));

  for (i = 0; i < 3; ++i)
    for (j = 0; j < 3; ++j)
      for (k = 0; k < 3; ++k)
        for (l = 0; l < 3; ++l)
          dft_dsig(i,j,k,l) = f1 * deltaFunc(i,k) * deltaFunc(j,l) - f2 * deltaFunc(i,j) * deltaFunc(k,l) - f3 * sig_dev(i,j) * sig_dev(k,l);

  dfd_dsig = dft_dsig;
  dresid_dsig = E_ijkl.invSymm() + dfd_dsig * flow_incr;
}

//Delta Function
Real
FiniteStrainPlasticMaterial::deltaFunc(unsigned int i, unsigned int j)
{
  if (i == j)
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

  nsize = _yield_stress_vector.size();
  data = _yield_stress_vector.data();

  if (data[0] > 0.0 || nsize % 2 > 0)//Error check for input inconsitency
    mooseError("Error in yield stress input: Should be a vector with eqv plastic strain and yield stress pair values.\n");

  unsigned int ind = 0;
  Real tol = 1e-8;

  while (ind<nsize)
  {
    if (std::abs(eqpe - data[ind]) < tol)
      return data[ind+1];

    if (ind + 2 < nsize)
    {
      if (eqpe > data[ind] && eqpe < data[ind+2])
        return data[ind+1]+(eqpe-data[ind])/(data[ind+2]-data[ind])*(data[ind+3]-data[ind+1]);
    }
    else
      return data[nsize-1];

    ind += 2;
  }

  return 0.0;
}

Real
FiniteStrainPlasticMaterial::getdYieldStressdPlasticStrain(Real eqpe)
{
  int nsize;
  Real *data;

  nsize = _yield_stress_vector.size();
  data = _yield_stress_vector.data();

  if (data[0] > 0.0 || nsize % 2 > 0)//Error check for input inconsitency
    mooseError("Error in yield stress input: Should be a vector with eqv plastic strain and yield stress pair values.\n");

  unsigned int ind = 0;

  while (ind < nsize)
  {
    if (ind + 2 < nsize)
    {
      if (eqpe >= data[ind] && eqpe < data[ind+2])
        return (data[ind+3]-data[ind+1])/(data[ind+2]-data[ind]);
    }
    else
      return 0.0;

    ind+=2;;
  }

  return 0.0;
}
