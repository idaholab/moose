#include "FiniteStrainCrystalPlasticity.h"
#include <cmath>

extern "C" void FORTRAN_CALL(dsyev) ( ... );

template<>
InputParameters validParams<FiniteStrainCrystalPlasticity>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();

  params.addRequiredParam<int >("nss", "Number of slip systems");
  params.addParam<std::vector<Real> >("gprops", "Initial values of slip system resistances");
  params.addParam<std::vector<Real> >("hprops", "Hardening properties");
  params.addParam<std::vector<Real> >("flowprops", "Parameters used in slip rate equations");
  params.addRequiredParam<std::string>("slip_sys_file_name", "Name of the file containing the slip system");
  params.addParam<std::string>("slip_sys_res_prop_file_name", "", "Name of the file containing the initial values of slip system resistances");
  params.addParam<std::string>("slip_sys_flow_prop_file_name", "", "Name of the file containing the values of slip rate equation parameters");
  params.addParam<std::string>("slip_sys_hard_prop_file_name", "", "Name of the file containing the values of hardness evolution parameters");
  params.addParam<std::string>("euler_angle_file_name", "", "Name of the file containing the euler angles");
  params.addParam<Real>("rtol", 1e-8, "Constitutive stress residue tolerance");
  params.addParam<Real>("gtol", 1e2, "Constitutive slip system resistance residual tolerance");
  params.addParam<Real>("slip_incr_tol", 2e-2, "Maximum allowable slip in an increment");
  params.addParam<unsigned int>("maxiter", 100 , "Maximum number of iterations for stress update");
  params.addParam<unsigned int>("maxitergss", 100 , "Maximum number of iterations for slip system resistance update");
  params.addParam<unsigned int>("num_slip_sys_flowrate_props", 2, "Number of flow rate properties for a slip system");//Used for reading flow rate parameters

  return params;
}

FiniteStrainCrystalPlasticity::FiniteStrainCrystalPlasticity(const std::string & name,
                                                             InputParameters parameters) :
    FiniteStrainMaterial(name, parameters),
    _nss(getParam<int>("nss")),
    _gprops(getParam<std::vector<Real> >("gprops")),
    _hprops(getParam<std::vector<Real> >("hprops")),
    _flowprops(getParam<std::vector<Real> >("flowprops")),
    _slip_sys_file_name(getParam<std::string>("slip_sys_file_name")),
    _slip_sys_res_prop_file_name(getParam<std::string>("slip_sys_res_prop_file_name")),
    _slip_sys_flow_prop_file_name(getParam<std::string>("slip_sys_flow_prop_file_name")),
    _slip_sys_hard_prop_file_name(getParam<std::string>("slip_sys_hard_prop_file_name")),
    _euler_angle_file_name(getParam<std::string>("euler_angle_file_name")),
    _rtol(getParam<Real>("rtol")),
    _gtol(getParam<Real>("gtol")),
    _slip_incr_tol(getParam<Real>("slip_incr_tol")),
    _maxiter(getParam<unsigned int>("maxiter")),
    _maxiterg(getParam<unsigned int>("maxitergss")),
    _num_slip_sys_flowrate_props(getParam<unsigned int>("num_slip_sys_flowrate_props")),
    _fp(declareProperty<RankTwoTensor>("fp")),//Plastic deformation gradient
    _fp_old(declarePropertyOld<RankTwoTensor>("fp")),//Plastic deformation gradient of previous increment
    _pk2(declareProperty<RankTwoTensor>("pk2")),//2nd Piola Kirchoff Stress
    _pk2_old(declarePropertyOld<RankTwoTensor>("pk2")),//2nd Piola Kirchoff Stress of previous increment
    _lag_e(declareProperty<RankTwoTensor>("lage")),//Elastic component of Lagrangian strain
    _gss(declareProperty<std::vector<Real> >("gss")),//Slip system resistances
    _gss_old(declarePropertyOld<std::vector<Real> >("gss")),//Slip system resistances of previous increment
    _acc_slip(declareProperty<Real>("acc_slip")),//Accumulated slip
    _acc_slip_old(declarePropertyOld<Real>("acc_slip")),//Accumulated alip of previous increment
    _update_rot(declareProperty<RankTwoTensor>("update_rot")),//Rotation tensor considering material rotation and crystal orientation
    _crysrot(declareProperty<RankTwoTensor>("crysrot")),//Rotation tensor considering crystal orientation
    _crysrot_old(declarePropertyOld<RankTwoTensor>("crysrot"))
{
}

void FiniteStrainCrystalPlasticity::initQpStatefulProperties()
{

  RealTensorValue rot;

  _stress[_qp].zero();

  _fp[_qp].zero();
  _fp[_qp].addIa(1.0);

  _pk2[_qp].zero();


  if (_slip_sys_res_prop_file_name.length()!=0)
    readFileInitSlipSysRes();
  else
    getInitSlipSysRes();

  if (_slip_sys_flow_prop_file_name.length()!=0)
    readFileFlowRateParams();
  else
    getFlowRateParams();

  if (_slip_sys_hard_prop_file_name.length()!=0)
    readFileHardnessParams();
  else
    getHardnessParams();


  _acc_slip[_qp] = 0.0;

  getEulerAngles();
  getEulerRotations();

  getSlipSystems();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot(i,j) = _crysrot[_qp](i,j);


  _elasticity_tensor[_qp].rotate(rot);
  _update_rot[_qp] = _crysrot[_qp];
}

/*
  Solves stress residual equation using NR.
  Updates slip system resistances iteratively.
 */
void FiniteStrainCrystalPlasticity::computeQpStress()
{
  unsigned int i;
  std::vector<Real> gss_prev(_nss), slip_incr(_nss), tau(_nss);
  RankTwoTensor pk2, fp_old_inv, fp_inv, resid, dpk2, sig, rot;
  Real gmax, gdiff, rnorm;
  RankFourTensor jac;
  Real slip_incr_max, fac;
  unsigned int iter, iterg, maxiter, maxiterg;

  maxiter = _maxiter;
  maxiterg = _maxiterg;

  _crysrot[_qp] = _crysrot_old[_qp];

  fp_inv.zero();
  fp_old_inv = _fp_old[_qp].inverse();

  gmax = 1.1 * _gtol;

  for (i = 0; i < _nss; ++i)
    _gss[_qp][i]=_gss_old[_qp][i];

  iterg=0;
  while (gmax > _gtol && iterg < maxiterg)//Check for slip system resistance update tolerance
  {
    iter = 0;
    pk2 = _pk2_old[_qp];
    calc_resid_jacob(pk2, sig, fp_old_inv, fp_inv, slip_incr, tau, resid, jac);//Calculate stress residual

    slip_incr_max = 0.0;

    for (i = 0; i < _nss; ++i)
      if (std::abs(slip_incr[i]) > slip_incr_max)
        slip_incr_max = std::abs(slip_incr[i]);

    fac = 1.0;
    if (slip_incr_max > _slip_incr_tol)//Check for allowable slip increment
    {
      fac = 0.1;
      _console << "slip_incr=" << _current_elem->id() << ' ' << _qp << ' ' << slip_incr_max << '\n';
      mooseError("Slip increment exceeds tolerance");
    }

    rnorm = resid.L2norm();
    // _console << "rnorm=" << iter << ' ' << rnorm << '\n';

    while (rnorm > _rtol && iter <  maxiter)//Check for stress residual tolerance
    {
      dpk2 = jac.invSymm() * resid;//Calculate stress increment
      pk2 = pk2 - dpk2 * fac;//Update stress
      calc_resid_jacob(pk2, sig, fp_old_inv, fp_inv, slip_incr, tau, resid, jac);//Calculate stress residual
      slip_incr_max=0.0;

      for (i = 0; i < _nss; ++i)
        if (std::abs(slip_incr[i]) > slip_incr_max)
          slip_incr_max = std::abs(slip_incr[i]);

      fac = 1.0;
      if (slip_incr_max > _slip_incr_tol)
      {
        fac = 0.1;
        _console << "slip_incr=" << _current_elem->id() << ' ' << _qp << ' ' << slip_incr_max << '\n';
        mooseError("Slip increment exceeds tolerance");
      }

      rnorm=resid.L2norm();

      iter++;
      // _console << "rnorm=" << iter << ' ' << rnorm << '\n';
    }

    if (iter == maxiter)
      mooseError("Stress Integration error\n");

    for (i = 0; i < _nss; ++i)
      gss_prev[i] = _gss[_qp][i];

    update_slip_system_resistance(slip_incr);//Update slip system resistance

    gmax = 0.0;
    for (i = 0; i < _nss; ++i)
    {
      gdiff = std::abs(gss_prev[i] - _gss[_qp][i]);//Calculate increment size

      if (gdiff > gmax)
        gmax = gdiff;
    }

    iterg++;
  }

  if (iterg == maxiterg)
  {
    _console << "gmax=" << gmax << '\n';
    mooseError("Hardness Integration error \n");
  }

  _fp[_qp] = fp_inv.inverse();
  _pk2[_qp] = pk2;
  _stress[_qp] = sig;


  rot = get_current_rotation(_dfgrd[_qp]);//Calculate material rotation

  _update_rot[_qp] = rot * _crysrot[_qp];
}

/*
  Calculate crystal rotation tensor from Euler Angles
 */
void
FiniteStrainCrystalPlasticity::getEulerRotations()
{
  Real phi1, phi, phi2;
  Real cp, cp1, cp2, sp, sp1, sp2;
  RankTwoTensor RT;
  Real pi = libMesh::pi;

  phi1 = _euler_angle_1 * (pi/180.0);
  phi = _euler_angle_2 * (pi/180.0);
  phi2 = _euler_angle_3 * (pi/180.0);

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

  _crysrot[_qp] = RT.transpose();
  _crysrot_old[_qp] = _crysrot[_qp];
}

/*
  Read slip systems from file
 */
void
FiniteStrainCrystalPlasticity::getSlipSystems()
{

  Real vec[3];
  std::ifstream fileslipsys;

  _mo.resize(_nss*3);
  _no.resize(_nss*3);

  fileslipsys.open(_slip_sys_file_name.c_str());

  if (!fileslipsys)
    mooseError("Can't open slip system input file");

  // Read the slip normal
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      fileslipsys >> vec[j];

    //Normalize the vectors
    Real mag;
    mag = std::pow(vec[0], 2.0) + std::pow(vec[1], 2.0) + std::pow(vec[2], 2.0);
    mag = std::pow(mag, 0.5);

    for (unsigned j = 0; j < 3; ++j)
      _no[i*3+j] = vec[j]/mag;
  }

  //Read the slip direction
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      fileslipsys >> vec[j];

    //Normalize the vectors
    Real mag;
    mag = std::pow(vec[0], 2.0) + std::pow(vec[1], 2.0) + std::pow(vec[2], 2.0);
    mag = std::pow(mag, 0.5);

    for (unsigned int j = 0; j < 3; ++j)
      _mo[i*3+j] = vec[j] / mag;//Slip direction
  }

  fileslipsys.close();

}

//Update slip system resistance. Overide to incorporate new slip system resistance laws
void
FiniteStrainCrystalPlasticity::update_slip_system_resistance(std::vector<Real> & slip_incr)
{

  updateGss(slip_incr);

}

/*
Old function to update slip system resistances.
Kept to avoid code break at computeQpstress
*/
void
FiniteStrainCrystalPlasticity::updateGss(std::vector<Real> & slip_incr)
{

  std::vector<Real> hb(_nss);
  Real qab,val;

  Real a = _hprops[4]; //Kalidindi

  _acc_slip[_qp]=_acc_slip_old[_qp];

  for (unsigned int i=0; i < _nss; i++)
    _acc_slip[_qp]=_acc_slip[_qp]+fabs(slip_incr[i]);

  val = std::cosh(_h0 * _acc_slip[_qp] / (_tau_sat - _tau_init)); //Karthik
  val = _h0 * std::pow(1.0/val,2.0); //Kalidindi

  for (unsigned int i = 0; i < _nss; i++)
    // hb[i]=val;
    hb[i] = _h0 * std::pow(1.0 - _gss[_qp][i] / _tau_sat, a);

  for (unsigned int i=0; i < _nss; i++)
  {
    _gss[_qp][i] = _gss_old[_qp][i];

    for (unsigned int j = 0; j < _nss; j++)
    {
      unsigned int iplane, jplane;
      iplane = i/3;
      jplane = j/3;

      if (iplane == jplane) //Kalidindi
        qab = 1.0;
      else
        qab = _r;

      _gss[_qp][i] = _gss[_qp][i] + qab * hb[j] * std::abs(slip_incr[j]);
    }
  }
}


//Calculates stress residual equation and jacobian
void
FiniteStrainCrystalPlasticity::calc_resid_jacob(const RankTwoTensor & pk2, RankTwoTensor & sig, const RankTwoTensor & fp_old_inv, RankTwoTensor & fp_inv,
                                                std::vector<Real> & slip_incr, std::vector<Real> & tau, RankTwoTensor & resid, RankFourTensor & jac)
{
  RankTwoTensor fe, ce, ee, iden, ce_pk2;
  RankTwoTensor eqv_slip_incr, pk2_new;
  std::vector<RankTwoTensor> s0(_nss), dtaudpk2(_nss), dfpinvdslip(_nss);
  RankTwoTensor temp2;
  std::vector<Real> dslipdtau(_nss);
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2, idenFour;
  RankFourTensor temp4;

  /*Calculate Residual*/
  iden.zero();
  iden.addIa(1.0);

  fe = _dfgrd[_qp] * fp_old_inv;

  ce = fe.transpose() * fe;
  ee = ce - iden;
  ee *= 0.5;

  ce_pk2 = ce * pk2;
  ce_pk2 = ce_pk2 / fe.det();
  // ce_pk2=(*pk2);//Approximation

  std::vector<Real> mo,no;

  mo.resize(3*_nss);
  no.resize(3*_nss);

  //Update slip direction and normal with crystal orientation
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < 3; j++)
    {
      mo[i*3+j] = 0.0;
      for (unsigned int k = 0; k < 3; ++k)
        mo[i*3+j] = mo[i*3+j] + _crysrot[_qp](j,k) * _mo[i*3+k];
    }

    for (unsigned int j = 0; j < 3; j++)
    {
      no[i*3+j] = 0.0;
      for (unsigned int k = 0; k < 3; k++)
        no[i*3+j] = no[i*3+j] + _crysrot[_qp](j,k) * _no[i*3+k];
    }
  }

  //Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        s0[i](j,k) = mo[i*3+j] * no[i*3+k];

    tau[i] = ce_pk2.doubleContraction(s0[i]);
  }

  getSlipIncrements(tau, slip_incr, dslipdtau); //Calculate dslip,dslipdtau

  eqv_slip_incr.zero();
  for (unsigned int i = 0; i < _nss; ++i)
    eqv_slip_incr += s0[i] * slip_incr[i];

  eqv_slip_incr = iden - eqv_slip_incr;
  fp_inv = fp_old_inv * eqv_slip_incr;
  fe = _dfgrd[_qp] * fp_inv;

  ce = fe.transpose() * fe;
  ee = ce - iden;
  ee *= 0.5;

  pk2_new = _elasticity_tensor[_qp] * ee;

  _lag_e[_qp] = _dfgrd[_qp].transpose() * _dfgrd[_qp] - iden;
  _lag_e[_qp] = _lag_e[_qp] * 0.5;

  resid = pk2 - pk2_new;
  /*End Calculate Residual*/
  /*Calculate Jacobian*/

  for (unsigned int i = 0; i < _nss; ++i)
  {
    dtaudpk2[i] = s0[i];
    dfpinvdslip[i] = -fp_old_inv * s0[i];
  }

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        dfedfpinv(i,j,k,j) = _dfgrd[_qp](i,k);

  deedfe.zero();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + fe(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + fe(k,i) * 0.5;
      }

  dfpinvdpk2.zero();
  for (unsigned int i = 0; i < _nss; ++i)
  {
    temp2 = dfpinvdslip[i] * dslipdtau[i];
    temp4 = outerProduct(temp2, dtaudpk2[i]);
    dfpinvdpk2 += temp4;
  }

  jac = _elasticity_tensor[_qp] * deedfe * dfedfpinv * dfpinvdpk2;

  for (unsigned int i = 0; i < 3 ; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      idenFour(i,j,i,j) = 1.0;

  jac = idenFour - jac;

  /*End Calculate Jacobian*/

  sig = fe * pk2 * fe.transpose();
  sig = sig / fe.det();
}

/*
Calculate slip increment. Override to modify.
 */
void
FiniteStrainCrystalPlasticity::getSlipIncrements(const std::vector<Real> & tau, std::vector<Real> & slip_incr, std::vector<Real> & dslipdtau)
{
  for (unsigned int i = 0; i < _nss; ++i)
    slip_incr[i] = _a0[i] * std::pow(std::abs(tau[i] / _gss[_qp][i]), 1.0 / _xm[i]) * copysign(1.0, tau[i]) * _dt;

  for (unsigned int i = 0; i < _nss; ++i)
    dslipdtau[i] = _a0[i] / _xm[i] * std::pow(std::abs(tau[i] / _gss[_qp][i]), 1.0 / _xm[i] - 1.0) / _gss[_qp][i] * _dt;
}

RankFourTensor FiniteStrainCrystalPlasticity::outerProduct(const RankTwoTensor & a, const RankTwoTensor & b)
{

  RankFourTensor result;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
          result(i,j,k,l) = a(i,j) * b(k,l);

  return result;
}

/*
Calls getMatRot to perform RU factorization of a tensor.
 */
RankTwoTensor
FiniteStrainCrystalPlasticity::get_current_rotation(const RankTwoTensor & a)
{

  return getMatRot(a);

}

/*
Performs RU factorization of a tensor
 */
RankTwoTensor
FiniteStrainCrystalPlasticity::getMatRot(const RankTwoTensor & a)
{

  RankTwoTensor rot;
  RankTwoTensor c, diag, evec;
  double cmat[3][3], w[3], work[10];
  int info;
  int nd = 3;
  int lwork = 10;

  c = a.transpose() * a;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      cmat[i][j] = c(i,j);

  FORTRAN_CALL(dsyev)("V","U",&nd,cmat,&nd,&w,&work,&lwork,&info);

  if (info != 0)
    mooseError("DSYEV function call in getMatRot function failed");

  diag.zero();

  for (unsigned int i = 0; i < 3; ++i)
    diag(i,i) = std::pow(w[i], 0.5);

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      evec(i,j) = cmat[i][j];

  rot = a * ((evec.transpose() * diag * evec).inverse());

  return rot;
}

void
FiniteStrainCrystalPlasticity::getEulerAngles()
{
  std::ifstream fileeuler;
  Real vec[3];
  int elemno;

  if (_euler_angle_file_name.length() != 0)
  {
    fileeuler.open(_euler_angle_file_name.c_str());

    if (!fileeuler)
      mooseError("Can't open euler angle input file");
    else
    {
      while (!fileeuler.eof())
      {
        fileeuler >> elemno;

        for (unsigned int i = 0; i < 3; ++i)
          fileeuler >> vec[i];

        if (static_cast<dof_id_type>(elemno-1) == _current_elem->id())
        {
          _euler_angle_1 = vec[0];
          _euler_angle_2 = vec[1];
          _euler_angle_3 = vec[2];

          fileeuler.close();
          return;
        }
      }

      fileeuler.close();
    }
  }
}

/*
  Calculates tangent moduli which is used for global solve
 */
void
FiniteStrainCrystalPlasticity::computeQpElasticityTensor()
{
  ElasticityTensorR4 tan_mod;
  RankTwoTensor fe, fp_inv, pk2fet, fepk2;
  RankFourTensor dfedf, deedfe, dsigdpk2dfe, temp;

  // Fill in the matrix stiffness material property
  _elasticity_tensor[_qp] = _Cijkl;

  fp_inv = _fp[_qp].inverse();
  fe = _dfgrd[_qp] * fp_inv;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int l = 0; l < 3; ++l)
        dfedf(i,j,i,l) = fp_inv(l,j);

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + fe(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + fe(k,i) * 0.5;
      }


  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
          temp(i,j,k,l) = fe(i,k) * fe(j,l);

  dsigdpk2dfe = temp * _elasticity_tensor[_qp] * deedfe;

  pk2fet = _pk2[_qp] * fe.transpose();
  fepk2 = fe * _pk2[_qp];

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int l = 0; l < 3; ++l)
      {
        tan_mod(i,j,i,l) = tan_mod(i,j,i,l) + pk2fet(l,j);
        tan_mod(i,j,j,l) = tan_mod(i,j,j,l) + fepk2(i,l);
      }

  tan_mod += dsigdpk2dfe;

  Real je = fe.det();
  tan_mod /= je;

  _Jacobian_mult[_qp] = tan_mod;
}

//Read initial slip system resistances  from .txt file. See test.
void
FiniteStrainCrystalPlasticity::readFileInitSlipSysRes()
{

  _gss[_qp].resize(_nss);
  _gss_old[_qp].resize(_nss);

  std::ifstream file;
  file.open(_slip_sys_res_prop_file_name.c_str());

  if (!file)
    mooseError("Can't open slip system resistance input file");


  for (unsigned int i = 0; i < _nss; ++i)
      file >> _gss[_qp][i];

  file.close();
}

//Read initial slip system resistances  from .i file
void
FiniteStrainCrystalPlasticity::getInitSlipSysRes()
{

  if (_gprops.size()==0)
    mooseError("Error in reading slip system resistance properties: Specify input in .i file or a slip_sys_res_prop_file_name");

  _gss[_qp].resize(_nss);
  _gss_old[_qp].resize(_nss);

  unsigned int ind;


  ind = 0;
  while (true)
  {

    Real vs,ve;
    unsigned int is, ie;

    vs=_gprops[ind++];
    ve=_gprops[ind++];

    if (vs != floor(vs) || ve != floor(ve))
      mooseError("Error in reading slip system resistances: Values specifying start and end number of slip system groups should be integer\n");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    for (unsigned int i = is; i <= ie; ++i)
      _gss[_qp][i-1] = _gprops[ind];

    if (ie == _nss)
      break;

    ind++;
  }

}

//Read flow rate parameters from .txt file. See test.
void
FiniteStrainCrystalPlasticity::readFileFlowRateParams()
{

  _a0.resize(_nss);
  _xm.resize(_nss);

  std::ifstream file;
  file.open(_slip_sys_flow_prop_file_name.c_str());

  if (!file)
    mooseError("Can't open slip system flow prop input file");

  std::vector<Real> vec;
  vec.resize(_num_slip_sys_flowrate_props);

  for (unsigned int i = 0; i < _nss; ++i)
    {
      for (unsigned int j = 0; j < _num_slip_sys_flowrate_props; ++j)
       file >> vec[j];

      _a0[i]=vec[0];
      _xm[i]=vec[1];
    }


  file.close();

}

//Read flow rate parameters from .i file
void
FiniteStrainCrystalPlasticity::getFlowRateParams()
{

  if (_flowprops.size()==0)
    mooseError("Error in reading flow rate  properties: Specify input in .i file or a slip_sys_flow_prop_file_name");


  unsigned int ind;


  _a0.resize(_nss);
  _xm.resize(_nss);

  ind = 0;
  while (true)
  {
    Real vs,ve;
    unsigned int is, ie;

    vs = _flowprops[ind++];
    ve = _flowprops[ind++];

    if (vs != floor(vs) || ve != floor(ve))
      mooseError("Error in reading flow props: Values specifying start and end number of slip system groups should be integer\n");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    for (unsigned int i = is; i <= ie; ++i)
    {
      _a0[i-1] = _flowprops[ind];
      _xm[i-1] = _flowprops[ind+1];
    }

    if (ie == _nss)
      break;

    ind += _num_slip_sys_flowrate_props;
  }

}

//Read hardness parameters from .txt file
void
FiniteStrainCrystalPlasticity::readFileHardnessParams()
{

}

//Read harness parameters from .i file
void
FiniteStrainCrystalPlasticity::getHardnessParams()
{

  if (_hprops.size()==0)
    mooseError("Error in reading hardness properties: Specify input in .i file or a slip_sys_hard_prop_file_name");

  _r = _hprops[0];
  _h0 = _hprops[1];
  _tau_init = _hprops[2];
  _tau_sat = _hprops[3];

}

