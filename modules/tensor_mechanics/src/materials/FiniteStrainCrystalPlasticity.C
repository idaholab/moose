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

  _tau.resize(_nss);
  _slip_incr.resize(_nss);
  _dslipdtau.resize(_nss);


  _mo.resize(_nss*LIBMESH_DIM);
  _no.resize(_nss*LIBMESH_DIM);

  _s0.resize(_nss);
}

void FiniteStrainCrystalPlasticity::initQpStatefulProperties()
{

  _stress[_qp].zero();

  _fp[_qp].zero();
  _fp[_qp].addIa(1.0);

  _pk2[_qp].zero();
  _pk2_tmp.zero();
  _acc_slip[_qp] = 0.0;

  initSlipSysProps();//Initializes slip system related properties

  getEulerAngles();
  getEulerRotations();

  getSlipSystems();

  initAdditionalProps();
}


/*Initializes slip system related properties like
slip system resistences, hardness, flow properties
 */
void
FiniteStrainCrystalPlasticity::initSlipSysProps()
{

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

}

//Read initial slip system resistances  from .txt file. See test.
void
FiniteStrainCrystalPlasticity::readFileInitSlipSysRes()
{

  _gss[_qp].resize(_nss);
  _gss_old[_qp].resize(_nss);
  _gss_tmp.resize(_nss);

  MooseUtils::checkFileReadable(_slip_sys_res_prop_file_name);

  std::ifstream file;
  file.open(_slip_sys_res_prop_file_name.c_str());

  for (unsigned int i = 0; i < _nss; ++i)
    file >> _gss[_qp][i];

  file.close();
}

//Read initial slip system resistances  from .i file
void
FiniteStrainCrystalPlasticity::getInitSlipSysRes()
{

  if (_gprops.size()==0)
    mooseError("FiniteStrainCrystalPLasticity: Error in reading slip system resistance properties: Specify input in .i file or a slip_sys_res_prop_file_name");

  _gss[_qp].resize(_nss);
  _gss_old[_qp].resize(_nss);
  _gss_tmp.resize(_nss);

  unsigned int ind;


  ind = 0;
  while (true)
  {

    Real vs,ve;
    unsigned int is, ie;

    vs=_gprops[ind++];
    ve=_gprops[ind++];

    if ( vs <= 0 || ve <= 0 )
      mooseError("FiniteStrainCrystalPLasticity: Indices in slip system resistance property read must be positive integers: is = " << vs << " ie = " << ve << "\n");

    if (vs != floor(vs) || ve != floor(ve))
      mooseError("FiniteStrainCrystalPLasticity: Error in reading slip system resistances: Values specifying start and end number of slip system groups should be integer\n");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    if ( is > ie )
      mooseError("FiniteStrainCrystalPLasticity: Start index is = " << is << " should be greater than end index ie = " << ie << " in slip system resistance property read \n");

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

  MooseUtils::checkFileReadable(_slip_sys_flow_prop_file_name);

  std::ifstream file;
  file.open(_slip_sys_flow_prop_file_name.c_str());

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
    mooseError("FiniteStrainCrystalPLasticity: Error in reading flow rate  properties: Specify input in .i file or a slip_sys_flow_prop_file_name");


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

    if ( vs <= 0 || ve <= 0 )
      mooseError("FiniteStrainCrystalPLasticity: Indices in flow rate parameter read must be positive integers: is = " << vs << " ie = " << ve << "\n");

    if (vs != floor(vs) || ve != floor(ve))
      mooseError("FiniteStrainCrystalPLasticity: Error in reading flow props: Values specifying start and end number of slip system groups should be integer\n");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    if ( is > ie )
      mooseError("FiniteStrainCrystalPLasticity: Start index is = " << is << " should be greater than end index ie = " << ie << " in flow rate parameter read \n");

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
    mooseError("FiniteStrainCrystalPLasticity: Error in reading hardness properties: Specify input in .i file or a slip_sys_hard_prop_file_name");

  _r = _hprops[0];
  _h0 = _hprops[1];
  _tau_init = _hprops[2];
  _tau_sat = _hprops[3];

}


void
FiniteStrainCrystalPlasticity::getEulerAngles()
{
  std::ifstream fileeuler;
  Real vec[LIBMESH_DIM];
  int elemno;

  if (_euler_angle_file_name.length() != 0)
  {

    MooseUtils::checkFileReadable(_euler_angle_file_name);

    fileeuler.open(_euler_angle_file_name.c_str());

    while (!fileeuler.eof())
    {
      fileeuler >> elemno;

      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
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

  Real vec[LIBMESH_DIM];
  std::ifstream fileslipsys;

  MooseUtils::checkFileReadable(_slip_sys_file_name);

  fileslipsys.open(_slip_sys_file_name.c_str());

  // Read the slip normal
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      fileslipsys >> vec[j];

    //Normalize the vectors
    Real mag;
    mag = std::pow(vec[0], 2.0) + std::pow(vec[1], 2.0) + std::pow(vec[2], 2.0);
    mag = std::pow(mag, 0.5);

    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      _no[i*LIBMESH_DIM+j] = vec[j]/mag;
  }

  //Read the slip direction
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      fileslipsys >> vec[j];

    //Normalize the vectors
    Real mag;
    mag = std::pow(vec[0], 2.0) + std::pow(vec[1], 2.0) + std::pow(vec[2], 2.0);
    mag = std::pow(mag, 0.5);

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      _mo[i*LIBMESH_DIM+j] = vec[j] / mag;//Slip direction
  }

  fileslipsys.close();

}

/*
Initialize addtional properties like rotation, rotated elasticity tensor, etc.
 */
void
FiniteStrainCrystalPlasticity::initAdditionalProps()
{

  RealTensorValue rot;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      rot(i,j) = _crysrot[_qp](i,j);

  _elasticity_tensor[_qp] = _Cijkl;

  _elasticity_tensor[_qp].rotate(rot);
  _update_rot[_qp] = _crysrot[_qp];

}


/*
  Solves stress residual equation using NR.
  Updates slip system resistances iteratively.
 */
void FiniteStrainCrystalPlasticity::computeQpStress()
{

  preSolveQp();

  solveQp();

  postSolveQp();

}


void
FiniteStrainCrystalPlasticity::preSolveQp()
{

  _crysrot[_qp] = _crysrot_old[_qp];

  _fp_old_inv = _fp_old[_qp].inverse();

  std::vector<Real> mo(LIBMESH_DIM*_nss),no(LIBMESH_DIM*_nss);

  //Update slip direction and normal with crystal orientation
  for (unsigned int i = 0; i < _nss; ++i)
  {

    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
    {
      mo[i*LIBMESH_DIM+j] = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        mo[i*LIBMESH_DIM+j] = mo[i*LIBMESH_DIM+j] + _crysrot[_qp](j,k) * _mo[i*LIBMESH_DIM+k];
    }

    for (unsigned int j = 0; j < LIBMESH_DIM; j++)
    {
      no[i*LIBMESH_DIM+j] = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; k++)
        no[i*LIBMESH_DIM+j] = no[i*LIBMESH_DIM+j] + _crysrot[_qp](j,k) * _no[i*LIBMESH_DIM+k];
    }
  }

  //Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < _nss; ++i)
  {
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        _s0[i](j,k) = mo[i*LIBMESH_DIM+j] * no[i*LIBMESH_DIM+k];

  }

}

void
FiniteStrainCrystalPlasticity::solveQp()
{

  preSolveStatevar();

  solveStatevar();

  postSolveStatevar();

}

void
FiniteStrainCrystalPlasticity::postSolveQp()
{

  RankTwoTensor iden;
  iden.addIa(1.0);

  _fp[_qp] = _fp_inv.inverse();

  _lag_e[_qp] = _dfgrd[_qp].transpose() * _dfgrd[_qp] - iden;
  _lag_e[_qp] = _lag_e[_qp] * 0.5;

  _pk2[_qp] = _pk2_tmp;
  _stress[_qp] = _fe * _pk2[_qp] * _fe.transpose()/_fe.det();


  RankTwoTensor rot;
  rot = get_current_rotation(_dfgrd[_qp]);//Calculate material rotation
  _update_rot[_qp] = rot * _crysrot[_qp];

  //Assign MaterialProperty values
  for (unsigned i = 0; i < _nss; ++i)
    _gss[_qp][i] = _gss_tmp[i];

  _acc_slip[_qp] = _accslip_tmp;
}

void
FiniteStrainCrystalPlasticity::preSolveStatevar()
{

  for (unsigned i = 0; i < _nss; ++i)
    _gss_tmp[i]=_gss_old[_qp][i];

}


void
FiniteStrainCrystalPlasticity::solveStatevar()
{

  Real gmax, gdiff;
  unsigned int iterg;
  std::vector<Real> gss_prev(_nss);

  gmax = 1.1 * _gtol;
  iterg = 0;

  while (gmax > _gtol && iterg < _maxiterg)//Check for slip system resistance update tolerance
  {

    preSolveStress();

    solveStress();

    postSolveStress();

    for (unsigned i = 0; i < _nss; ++i)
      gss_prev[i] = _gss_tmp[i];

    update_slip_system_resistance();//Update slip system resistance

    gmax = 0.0;
    for (unsigned i = 0; i < _nss; ++i)
    {
      gdiff = std::abs(gss_prev[i] - _gss_tmp[i]);//Calculate increment size

      if (gdiff > gmax)
        gmax = gdiff;
    }

    iterg++;

  }

  if (iterg == _maxiterg)
  {
    mooseError("FiniteStrainCrystalPLasticity: Hardness Integration error gmax" << gmax << "\n");
  }

}

void
FiniteStrainCrystalPlasticity::postSolveStatevar()
{



}

void
FiniteStrainCrystalPlasticity::preSolveStress()
{

  _pk2_tmp = _pk2_old[_qp];

}

void
FiniteStrainCrystalPlasticity::solveStress()
{

  Real slip_incr_max;
  unsigned int iter = 0;
  RankTwoTensor resid, dpk2;
  RankFourTensor jac;

  calc_resid_jacob(resid, jac);//Calculate stress residual

  slip_incr_max = 0.0;

  for (unsigned i = 0; i < _nss; ++i)
    if (std::abs(_slip_incr[i]) > slip_incr_max)
      slip_incr_max = std::abs(_slip_incr[i]);

  Real fac = 1.0;

  if (slip_incr_max > _slip_incr_tol)//Check for allowable slip increment
  {
    fac = 0.1;
    mooseError("FiniteStrainCrystalPLasticity: Slip increment exceeds tolerance - Element number" << _current_elem->id() << " Gauss point = " << _qp << " slip_incr_max = " << slip_incr_max << "\n");
  }

  Real rnorm = resid.L2norm();
  // _console << "rnorm=" << iter << ' ' << rnorm << '\n';

  while (rnorm > _rtol && iter <  _maxiter)//Check for stress residual tolerance
  {
    dpk2 = jac.invSymm() * resid;//Calculate stress increment
    _pk2_tmp = _pk2_tmp - dpk2 * fac;//Update stress

    calc_resid_jacob(resid, jac);//Calculate stress residual

    slip_incr_max=0.0;

    for (unsigned i = 0; i < _nss; ++i)
      if (std::abs(_slip_incr[i]) > slip_incr_max)
        slip_incr_max = std::abs(_slip_incr[i]);

    fac = 1.0;
    if (slip_incr_max > _slip_incr_tol)
    {
      fac = 0.1;
      mooseError("FiniteStrainCrystalPLasticity: Slip increment exceeds tolerance - Element number" << _current_elem->id() << " Gauss point = " << _qp << " slip_incr_max = " << slip_incr_max << "\n");

    }

    rnorm=resid.L2norm();

    iter++;

  }


}

void
FiniteStrainCrystalPlasticity::postSolveStress()
{

}


//Update slip system resistance. Overide to incorporate new slip system resistance laws
void
FiniteStrainCrystalPlasticity::update_slip_system_resistance()
{

  updateGss();

}

/*
Old function to update slip system resistances.
Kept to avoid code break at computeQpstress
*/
void
FiniteStrainCrystalPlasticity::updateGss()
{

  std::vector<Real> hb(_nss);
  Real qab,val;

  Real a = _hprops[4]; //Kalidindi

  _accslip_tmp=_acc_slip_old[_qp];

  for (unsigned int i=0; i < _nss; i++)
    _accslip_tmp=_accslip_tmp+fabs(_slip_incr[i]);

  val = std::cosh(_h0 * _accslip_tmp / (_tau_sat - _tau_init)); //Karthik
  val = _h0 * std::pow(1.0/val,2.0); //Kalidindi

  for (unsigned int i = 0; i < _nss; i++)
    hb[i] = _h0 * std::pow(std::abs(1.0 - _gss_tmp[i]/_tau_sat),a) * copysign(1.0,1.0-_gss_tmp[i]/_tau_sat);

  for (unsigned int i=0; i < _nss; i++)
  {
    _gss_tmp[i] = _gss_old[_qp][i];

    for (unsigned int j = 0; j < _nss; j++)
    {
      unsigned int iplane, jplane;
      iplane = i/3;
      jplane = j/3;

      if (iplane == jplane) //Kalidindi
        qab = 1.0;
      else
        qab = _r;

      _gss_tmp[i] = _gss_tmp[i] + qab * hb[j] * std::abs(_slip_incr[j]);
    }
  }
}


//Calculates stress residual equation and jacobian
void
FiniteStrainCrystalPlasticity::calc_resid_jacob( RankTwoTensor & resid, RankFourTensor & jac)
{

  calcResidual( resid );
  calcJacobian( jac );

}


void
FiniteStrainCrystalPlasticity::calcResidual( RankTwoTensor &resid )
{

  RankTwoTensor iden, ce, ee, ce_pk2, eqv_slip_incr, pk2_new;


  iden.zero();
  iden.addIa(1.0);

  _fe = _dfgrd[_qp] * _fp_old_inv;


  ce = _fe.transpose() * _fe;
  ce_pk2 = ce * _pk2_tmp;
  ce_pk2 = ce_pk2 / _fe.det();

  //Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < _nss; ++i)
    _tau[i] = ce_pk2.doubleContraction(_s0[i]);


  getSlipIncrements(); //Calculate dslip,dslipdtau

  eqv_slip_incr.zero();
  for (unsigned int i = 0; i < _nss; ++i)
    eqv_slip_incr += _s0[i] * _slip_incr[i];

  eqv_slip_incr = iden - eqv_slip_incr;
  _fp_inv = _fp_old_inv * eqv_slip_incr;
  _fe = _dfgrd[_qp] * _fp_inv;


  ce = _fe.transpose() * _fe;
  ee = ce - iden;
  ee *= 0.5;

  pk2_new = _elasticity_tensor[_qp] * ee;

  resid = _pk2_tmp - pk2_new;

}

void
FiniteStrainCrystalPlasticity::calcJacobian( RankFourTensor &jac )
{

  RankTwoTensor temp2;
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2, idenFour;
  RankFourTensor temp4;
  std::vector<RankTwoTensor> dtaudpk2(_nss), dfpinvdslip(_nss);

  for (unsigned int i = 0; i < _nss; ++i)
  {
    dtaudpk2[i] = _s0[i];
    dfpinvdslip[i] = - _fp_old_inv * _s0[i];
  }

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        dfedfpinv(i,j,k,j) = _dfgrd[_qp](i,k);

  deedfe.zero();

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + _fe(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + _fe(k,i) * 0.5;
      }

  dfpinvdpk2.zero();
  for (unsigned int i = 0; i < _nss; ++i)
  {
    temp2 = dfpinvdslip[i] * _dslipdtau[i];
    temp4 = outerProduct(temp2, dtaudpk2[i]);
    dfpinvdpk2 += temp4;
  }

  jac = _elasticity_tensor[_qp] * deedfe * dfedfpinv * dfpinvdpk2;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      idenFour(i,j,i,j) = 1.0;

  jac = idenFour - jac;

}

/*
Calculate slip increment,dslipdtau. Override to modify.
 */
void
FiniteStrainCrystalPlasticity::getSlipIncrements()
{
  for (unsigned int i = 0; i < _nss; ++i)
    _slip_incr[i] = _a0[i] * std::pow(std::abs(_tau[i] / _gss_tmp[i]), 1.0 / _xm[i]) * copysign(1.0, _tau[i]) * _dt;

  for (unsigned int i = 0; i < _nss; ++i)
    _dslipdtau[i] = _a0[i] / _xm[i] * std::pow(std::abs(_tau[i] / _gss_tmp[i]), 1.0 / _xm[i] - 1.0) / _gss_tmp[i] * _dt;
}

RankFourTensor FiniteStrainCrystalPlasticity::outerProduct(const RankTwoTensor & a, const RankTwoTensor & b)
{

  RankFourTensor result;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
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
  double cmat[LIBMESH_DIM][LIBMESH_DIM], w[LIBMESH_DIM], work[10];
  int info;
  int nd = LIBMESH_DIM;
  int lwork = 10;

  c = a.transpose() * a;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      cmat[i][j] = c(i,j);

  FORTRAN_CALL(dsyev)("V","U",&nd,cmat,&nd,&w,&work,&lwork,&info);

  if (info != 0)
    mooseError("FiniteStrainCrystalPLasticity: DSYEV function call in getMatRot function failed");

  diag.zero();

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    diag(i,i) = std::pow(w[i], 0.5);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      evec(i,j) = cmat[i][j];

  rot = a * ((evec.transpose() * diag * evec).inverse());

  return rot;
}





/*
  Calculates tangent moduli which is used for global solve
 */
void
FiniteStrainCrystalPlasticity::computeQpElasticityTensor()
{
  ElasticityTensorR4 tan_mod;
  RankTwoTensor pk2fet, fepk2, fp_inv_tmp(_fp_old[_qp].inverse()), _fe_tmp(_dfgrd[_qp]*fp_inv_tmp);
  RankFourTensor dfedf, deedfe, dsigdpk2dfe, temp;

  // Fill in the matrix stiffness material property

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        dfedf(i,j,i,l) = fp_inv_tmp(l,j);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i,j,k,i) = deedfe(i,j,k,i) + _fe_tmp(k,j) * 0.5;
        deedfe(i,j,k,j) = deedfe(i,j,k,j) + _fe_tmp(k,i) * 0.5;
      }


  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
          temp(i,j,k,l) = _fe_tmp(i,k) * _fe_tmp(j,l);

  dsigdpk2dfe = temp * _elasticity_tensor[_qp] * deedfe;

  pk2fet = _pk2_old[_qp] * _fe_tmp.transpose();
  fepk2 = _fe_tmp * _pk2_old[_qp];

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      {
        tan_mod(i,j,i,l) = tan_mod(i,j,i,l) + pk2fet(l,j);
        tan_mod(i,j,j,l) = tan_mod(i,j,j,l) + fepk2(i,l);
      }

  tan_mod += dsigdpk2dfe;

  Real je = _fe_tmp.det();

  if ( je > 0.0 )
    tan_mod /= je;

  _Jacobian_mult[_qp] = tan_mod;
}
