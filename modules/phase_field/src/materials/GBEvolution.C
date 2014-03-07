#include "GBEvolution.h"

template<>
InputParameters validParams<GBEvolution>()
{
  InputParameters params = validParams<Material>();

  params.addParam<Real>("temp", 300,"Constant temperature in Kelvin");
  params.addCoupledVar("T", "Temperature in Kelvin");
  params.addParam<Real>("f0s", 0.125,"The GB energy constant ");
  params.addRequiredParam<Real>("wGB","Diffuse GB width in nm ");
  params.addParam<Real>("length_scale",1.0e-9,"Length scale in m, where default is nm");
  params.addParam<Real>("time_scale", 1.0e-9,"Time scale in s, where default is ns");
  params.addParam<Real>("GBMobility",-1,"GB mobility input in m^4/(J*s), that overrides the temperature dependent calculation");
  params.addParam<Real>("GBmob0",0,"Grain boundary mobility prefactor in m^4/(J*s)");
  params.addParam<Real>("Q",0,"Grain boundary migration activation energy in eV");
  params.addRequiredParam<Real>("GBenergy","Grain boundary energy in J/m^2");
  params.addParam<Real>("molar_volume",24.62e-6,"Molar volume in m^3/mol, needed for temperature gradient driving force");

  return params;
}


GBEvolution::GBEvolution(const std::string & name,
                 InputParameters parameters)
    :Material(name, parameters),
   _temp(getParam<Real>("temp")),
   //_cg(coupledValue("cg")),
   _f0s(getParam<Real>("f0s")),
   _wGB(getParam<Real>("wGB")),
   _length_scale(getParam<Real>("length_scale")),
   _time_scale(getParam<Real>("time_scale")),
   _GBmob0(getParam<Real>("GBmob0")),
   _Q(getParam<Real>("Q")),
   _GBenergy(getParam<Real>("GBenergy")),
   _has_T(isCoupled("T")),
   _GBMobility(getParam<Real>("GBMobility")),
   _molar_vol(getParam<Real>("molar_volume")),
   _T(_has_T ? &coupledValue("T") : NULL),
   _sigma(declareProperty<Real>("sigma")),
   _M_GB(declareProperty<Real>("M_GB")),
   _kappa(declareProperty<Real>("kappa_op")),
   _gamma(declareProperty<Real>("gamma_asymm")),
   _L(declareProperty<Real>("L")),
   _l_GB(declareProperty<Real>("l_GB")),
   _mu(declareProperty<Real>("mu")),
   _entropy_diff(declareProperty<Real>("entropy_diff")),
   _molar_volume(declareProperty<Real>("molar_volume")),
   _act_wGB(declareProperty<Real>("act_wGB")),
   _tgrad_corr_mult(declareProperty<Real>("tgrad_corr_mult")),
   _kb(8.617343e-5) //Boltzmann constant in eV/K
{
  //Moose::out << "GB mob = " << _GBMobility << ", GBmob0 = " << _GBmob0 << std::endl;

  if (_GBMobility == -1 && _GBmob0 == 0)
    mooseError("Either a value for GBMobility or for GBmob0 and Q must be provided");
}

void
GBEvolution::computeProperties()
{
  Real M0 = _GBmob0;

  Real JtoeV = 6.24150974e18;// joule to eV conversion

  Real T = 0.0;

  M0 *= _time_scale/(JtoeV*(_length_scale*_length_scale*_length_scale*_length_scale));//Convert to lengthscale^4/(eV*timescale);

  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    if (_has_T)
      T = (*_T)[qp];
    else
      T = _temp;

    _sigma[qp] = _GBenergy*JtoeV*(_length_scale*_length_scale);// eV/nm^2

    if (_GBMobility < 0)
      _M_GB[qp] = M0*std::exp(-_Q/(_kb*T));
    else
      _M_GB[qp] = _GBMobility*_time_scale/(JtoeV*(_length_scale*_length_scale*_length_scale*_length_scale)); //Convert to lengthscale^4/(eV*timescale)

    _l_GB[qp] = _wGB; //in the length scale of the system

    _L[qp] = 4.0/3.0*_M_GB[qp]/_l_GB[qp];

    _kappa[qp] = 3.0/4.0*_sigma[qp]*_l_GB[qp];

    _gamma[qp] = 1.5;

    _mu[qp] = 3.0/4.0*1/_f0s*_sigma[qp]/_l_GB[qp];

    _entropy_diff[qp] = 8.0e3*JtoeV; //J/(K mol) converted to eV(K mol)

    _molar_volume[qp] = _molar_vol/(_length_scale*_length_scale*_length_scale); //m^3/mol converted to ls^3/mol

    _act_wGB[qp] = 0.5e-9/_length_scale;

    _tgrad_corr_mult[qp] = _mu[qp]*9.0/8.0;

  }
}
