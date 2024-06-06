
// 5D Gaussian anisotropy material object
// Material properties adding anysotroopy to ε [or also called k] and m;
// L  is isotropic and computed from a given mobility.


#include "EPSandManisoGAUSS.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"

#include <fstream>
#include <iostream>
#include <vector>


registerMooseObject("PhaseFieldApp", EPSandManisoGAUSS);

InputParameters
EPSandManisoGAUSS::validParams()
{
  InputParameters params = ADMaterial::validParams();

  params.addClassDescription("Material to provide an anisotropy field and its derivatives wrt to gradient of order parameter");

  params.addRequiredParam<MaterialPropertyName>("eps_name", "The name of the aniso property produced by this class.");
  params.addRequiredParam<MaterialPropertyName>("epsenergy_name", "The name of the aniso property produced by this class.");

  params.addRequiredParam<MaterialPropertyName>("depsdx_name", "Derivative of  eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("depsdy_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("depsdz_name", "Derivative of  eps in z direction");


  params.addRequiredParam<MaterialPropertyName>("depsdx_EN_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("depsdy_EN_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("depsdz_EN_name", "Derivative of  eps in z direction");


  params.addRequiredParam<MaterialPropertyName>("VwlibR_name", "The name of the library boundary normal in simulation Reference frame R");
  params.addRequiredParam<MaterialPropertyName>("VxlibR_name", "The name of the library boundary normal in simulation Reference frame R in x direction");
  params.addRequiredParam<MaterialPropertyName>("VylibR_name", "The name of the library boundary normal in simulation Reference frame R in y direction");
  params.addRequiredParam<MaterialPropertyName>("VzlibR_name", "The name of the library boundary normal in simulation Reference frame R in z direction");
  params.addRequiredParam<MaterialPropertyName>("Vx_name", "The name of normmaliized gradiient in x directtion");
  params.addRequiredParam<MaterialPropertyName>("Vy_name", "The name of normmaliized gradiient in y directtion");
  params.addRequiredParam<MaterialPropertyName>("Vz_name", "The name of normmaliized gradiient in z directtion");


  params.addRequiredParam<MaterialPropertyName>("dmdx_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("dmdy_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("dmdz_name", "Derivative of  eps in z direction");
  params.addRequiredParam<MaterialPropertyName>("dmdx_EN_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("dmdy_EN_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("dmdz_EN_name", "Derivative of  eps in z direction");
  params.addRequiredParam<MaterialPropertyName>("dmdxplus_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("dmdyplus_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("dmdzplus_name", "Derivative of  eps in z direction");
  params.addRequiredParam<MaterialPropertyName>("dmdxplus_EN_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("dmdyplus_EN_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("dmdzplus_EN_name", "Derivative of  eps in z direction");


  params.addRequiredParam<MaterialPropertyName>("depsdxplus_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("depsdyplus_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("depsdzplus_name", "Derivative of  eps in z direction");
  params.addRequiredParam<MaterialPropertyName>("depsdxplus_EN_name", "Derivative of eps in x direction");
  params.addRequiredParam<MaterialPropertyName>("depsdyplus_EN_name", "Derivative of  eps in y direction");
  params.addRequiredParam<MaterialPropertyName>("depsdzplus_EN_name", "Derivative of  eps in z direction");

  params.addRequiredParam<MaterialPropertyName>("S_switch_name", "The name of switch");

  params.addRequiredParam<MaterialPropertyName>("epsCalc_name", "The name of the calculated eps");
  params.addRequiredParam<MaterialPropertyName>("mCalc_name", "The name of the calculated m");

  params.addRequiredParam<MaterialPropertyName>("m_name", "The name of the aniso property m");
  params.addRequiredParam<MaterialPropertyName>("m_energy_name", "The name of the aniso property m");

  params.addRequiredParam<MaterialPropertyName>("L_name", "The name of the mobility.");
  params.addRequiredParam<MaterialPropertyName>("L_energy_name", "The name of the mobility.");

  params.addRequiredParam<MaterialPropertyName>("sigma_name", "Grain boundary energy");
  params.addRequiredParam<MaterialPropertyName>("sigmaORIUNIT_name", "Grain boundary energy");

  params.addRequiredParam<MaterialPropertyName>("qwg_name", "w component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qxg_name", "x component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qyg_name", "y component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qzg_name", "z component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("epsbar_name", "The name of the average anisotropy");
  params.addRequiredParam<MaterialPropertyName>("epsmin2grains_name", "The name of the 2grains minimum epsilon");

  params.addRequiredParam<MaterialPropertyName>("TotGauss_name", "The total gaussian to add/remove");

  params.addRequiredParam<FileName>("Library_file_name",  "Name of the file containing library quaternion data");

  params.addRequiredParam<FileName>("Quaternion_file_name",  "Name of the file containing anisotropy data");

  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");

  params.addRequiredParam<Real>("Gaussian_Tolerance", "value of Gaussian_Tolerance");

  params.addRequiredParam<Real>("gamma", "value of gamma");

  params.addRequiredParam<Real>("sigmaBASE", "value of sigmaBASE energy");

  params.addRequiredParam<Real>("lgb", "value of lgb");

  params.addRequiredParam<Real>("alphaswitch", "acceptable range of angle from vbalib");

  params.addRequiredParam<Real>("betaswitch", "acceptable range of angle from thetabalib");

  params.addRequiredParam<int>("libnum", "number of misorientations in the library");

  params.addRequiredParam<Real>("amplitudeScale", "amplitudeScale of gaussians");

  params.addRequiredParam<Real>("sharpness", "sharpness of gaussians");

  params.addRequiredParam<UserObjectName>("grain_tracker","graintracker UserObject");

  params.addRequiredParam<bool>("ADDGaussian", "If this is true gaussians add to eps_base, else subtract");

  params.addParam<Real>("T", 273, "Temperature in Kelvin");

  params.addParam<Real>("length_scale", 1.0e-9, "conversion for 1/m to 1/nm");

  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns");

  params.addParam<Real>("JtoeV", 6.24150974e18, "Joule to eV conversions");

  params.addParam<Real>("kb", 8.617343e-5, "Boltzmann constant in eV/K");

  params.addParam<Real>("GBMobility",1,"GB mobility input in m^4/(J*s), that overrides the temperature dependent calculation");

  params.addParam<Real>("Mob", 0, "Grain boundary mobility prefactor in m^4/(J*s)");

  params.addParam<Real>("Q", 0, "Grain boundary migration activation energy in eV");

  params.addParam<Real>("BoundaryNormal", 0, "if = 0,the boundary normal in the simulation reference frame is computed using the orientations quaternions, else it is taken directly from the minima library file");


  return params;
}

EPSandManisoGAUSS::EPSandManisoGAUSS(const InputParameters & parameters)
  : ADMaterial(parameters),

    _eps(declareADProperty<Real>(getParam<MaterialPropertyName>("eps_name"))),
    _epsenergy(declareProperty<Real>(getParam<MaterialPropertyName>("epsenergy_name"))),
    _depsdx(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdx_name"))),
    _depsdy(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdy_name"))),
    _depsdz(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdz_name"))),


    _depsdx_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdx_EN_name"))),
    _depsdy_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdy_EN_name"))),
    _depsdz_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdz_EN_name"))),

    _epsCalc(declareADProperty<Real>(getParam<MaterialPropertyName>("epsCalc_name"))),
    _mCalc(declareADProperty<Real>(getParam<MaterialPropertyName>("mCalc_name"))),

    _depsdxplus(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdxplus_name"))),
    _depsdyplus(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdyplus_name"))),
    _depsdzplus(declareADProperty<Real>(getParam<MaterialPropertyName>("depsdzplus_name"))),
    _depsdxplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdxplus_EN_name"))),
    _depsdyplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdyplus_EN_name"))),
    _depsdzplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("depsdzplus_EN_name"))),

    _S_switch(declareADProperty<Real>(getParam<MaterialPropertyName>("S_switch_name"))),

    _VwlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VwlibR_name"))),
    _VxlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VxlibR_name"))),
    _VylibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VylibR_name"))),
    _VzlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VzlibR_name"))),

    _Vx(declareADProperty<Real>(getParam<MaterialPropertyName>("Vx_name"))),
    _Vy(declareADProperty<Real>(getParam<MaterialPropertyName>("Vy_name"))),
    _Vz(declareADProperty<Real>(getParam<MaterialPropertyName>("Vz_name"))),

    _dmdx(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdx_name"))),
    _dmdy(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdy_name"))),
    _dmdz(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdz_name"))),
    _dmdx_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdx_EN_name"))),
    _dmdy_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdy_EN_name"))),
    _dmdz_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdz_EN_name"))),
    _dmdxplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdxplus_name"))),
    _dmdyplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdyplus_name"))),
    _dmdzplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dmdzplus_name"))),
    _dmdxplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdxplus_EN_name"))),
    _dmdyplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdyplus_EN_name"))),
    _dmdzplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dmdzplus_EN_name"))),

    _ADDGaussian(getParam<bool>("ADDGaussian")),

    _m(declareADProperty<Real>(getParam<MaterialPropertyName>("m_name"))),
    _m_energy(declareProperty<Real>(getParam<MaterialPropertyName>("m_energy_name"))),

    _L(declareADProperty<Real>(getParam<MaterialPropertyName>("L_name"))),
    _L_energy(declareProperty<Real>(getParam<MaterialPropertyName>("L_energy_name"))),

    _sigma(declareADProperty<Real>(getParam<MaterialPropertyName>("sigma_name"))),
    _sigmaORIUNIT(declareADProperty<Real>(getParam<MaterialPropertyName>("sigmaORIUNIT_name"))),

    _qwg(declareADProperty<Real>(getParam<MaterialPropertyName>("qwg_name"))),

    _qxg(declareADProperty<Real>(getParam<MaterialPropertyName>("qxg_name"))),

    _qyg(declareADProperty<Real>(getParam<MaterialPropertyName>("qyg_name"))),

    _qzg(declareADProperty<Real>(getParam<MaterialPropertyName>("qzg_name"))),

    _epsbar(declareADProperty<Real>(getParam<MaterialPropertyName>("epsbar_name"))),
    _epsmin2grains(declareADProperty<Real>(getParam<MaterialPropertyName>("epsmin2grains_name"))),

    _TotGauss(declareADProperty<Real>(getParam<MaterialPropertyName>("TotGauss_name"))),

    _sigmaBASE(getParam<Real>("sigmaBASE")),

    _Gaussian_Tolerance(getParam<Real>("Gaussian_Tolerance")),

    _gamma(getParam<Real>("gamma")),

    _lgb(getParam<Real>("lgb")),

    _alphaswitch(getParam<Real>("alphaswitch")),

    _betaswitch(getParam<Real>("betaswitch")),

    _libnum(getParam<int>("libnum")),

    _amplitudeScale(getParam<Real>("amplitudeScale")),

    _sharpness(getParam<Real>("sharpness")),

    _Library_file_name(getParam<FileName>("Library_file_name")),

    _Quaternion_file_name(getParam<FileName>("Quaternion_file_name")),

    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),

    _length_scale(getParam<Real>("length_scale")),

    _time_scale(getParam<Real>("time_scale")),

    _JtoeV(getParam<Real>("JtoeV")),

    _GBMobility(getParam<Real>("GBMobility")),

    _Mob(getParam<Real>("Mob")),

    _Q(getParam<Real>("Q")),

    _T(getParam<Real>("T")),

    _kb(getParam<Real>("kb")),

    _BoundaryNormal(getParam<Real>("BoundaryNormal")),

    _op_num(coupledComponents("v")),

    _vals(adCoupledValues("v")),

    _grad_vals(adCoupledGradients("v"))


    {
      if (_op_num == 0)
        paramError("op_num", "op_num must be greater than 0");



      if (_GBMobility == 1 && _Mob == 0 && _Q == 0)
        mooseError("Enter GBMobility or Mob and Q");
    }


void
EPSandManisoGAUSS::computeQpProperties()
{



        _grain_num = _grain_tracker.getTotalFeatureCount();

        const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());


        //************************ Read Quaternions file *************************************
        std::ifstream File1;
        File1.open(_Quaternion_file_name.c_str());

      	std::vector<double> qwR;
        std::vector<double> qxR;
        std::vector<double> qyR;
        std::vector<double> qzR;

      	double QuatW;
        double QuatX;
        double QuatY;
        double QuatZ;

      	while (!File1.eof())
      	{
      		File1 >> QuatW >> QuatX >> QuatY >> QuatZ;
      		qwR.push_back(QuatW);
          qxR.push_back(QuatX);
          qyR.push_back(QuatY);
          qzR.push_back(QuatZ);
      	}

      	File1.close();


        //************************ Read Library file *************************************
        std::ifstream File2;
        File2.open(_Library_file_name.c_str());

        std::vector<double> vbalibx;
        std::vector<double> vbaliby;
        std::vector<double> vbalibz;
        std::vector<double> thetabalib;
        std::vector<double> miubalibx;
        std::vector<double> miubaliby;
        std::vector<double> miubalibz;
        std::vector<double> MinimaEnergy;

        double Lib1;
        double Lib2;
        double Lib3;
        double Lib4;
        double Lib5;
        double Lib6;
        double Lib7;
        double Lib8;

        while (!File2.eof())
        {
          File2 >> Lib1 >> Lib2 >> Lib3 >> Lib4 >> Lib5 >> Lib6 >> Lib7 >> Lib8;
          vbalibx.push_back(Lib1);
          vbaliby.push_back(Lib2);
          vbalibz.push_back(Lib3);
          thetabalib.push_back(Lib4);
          miubalibx.push_back(Lib5);
          miubaliby.push_back(Lib6);
          miubalibz.push_back(Lib7);
          MinimaEnergy.push_back(Lib8);
        }

        File2.close();


        //******************************* Grain quaternion outputs ******************

        for (unsigned int h = 0; h < _grain_num; ++h)
        {
          for (unsigned int s = 0; s < _op_num; ++s)
          {
            if (h == op_to_grains[s])
            {
                _qwg[_qp] = qwR[h];
                _qxg[_qp] = qxR[h];
                _qyg[_qp] = qyR[h];
                _qzg[_qp] = qzR[h];
            }
          }
        }

       //**************************************************************************


        ADReal sumeps = 0.0;
        ADReal sumepsx = 0.0;
        ADReal sumepsy = 0.0;
        ADReal sumepsz = 0.0;

        ADReal sumepsxplus = 0.0;
        ADReal sumepsyplus = 0.0;
        ADReal sumepszplus = 0.0;

        ADReal aniso = 0.0;
        ADReal anisox = 0.0;
        ADReal anisoy = 0.0;
        ADReal anisoz = 0.0;

        ADReal anisoxplus = 0.0;
        ADReal anisoyplus = 0.0;
        ADReal anisozplus = 0.0;

        ADReal Val = 0.0;
        ADReal Val1 = 0.0;
        ADReal sumval = 0.0;

        ADReal f0saddle = 0.0;
        ADReal g_gamma = 0.0;

        ADReal eps_base = 0.0;
        ADReal TotGaussValue = 0.0;
        ADReal SumTotGaussValue = 0.0;
        ADReal Totepsbar = 0.0;

        ADReal eps_min = 0.0;
        ADReal Totepsmin = 0.0;



        f0saddle = 0.124961;

        g_gamma = 0.471404;

        // eps_base = ( (_sigmaBASE *_JtoeV * _length_scale * _length_scale) * _lgb *  (std::sqrt(f0saddle)) ) / (g_gamma) ;
        eps_base = (3.0/4.0) * (_sigmaBASE *_JtoeV * _length_scale * _length_scale) * _lgb ;

        //********************** sum (etai*etaj)^2 at each quadrature point **************************

        for (unsigned int m = 0; m < _op_num - 1; ++m)
        {
          for (unsigned int n = m + 1; n < _op_num; ++n) // m<n
          {
            Val1 = (((*_vals[m])[_qp]) * ((*_vals[m])[_qp])) * (((*_vals[n])[_qp]) * ((*_vals[n])[_qp]));
            sumval += Val1;
          }
        }

        //********************** Main anisotopy calculation **************

        for (unsigned int b = 0; b < _grain_num - 1; ++b)
        {

          for (unsigned int m = 0; m < _op_num - 1; ++m)
          {

            for (unsigned int a = b + 1; a < _grain_num; ++a)
            {

              for (unsigned int n = m + 1; n < _op_num; ++n)
              {

                if (b == op_to_grains[m])
                {

                  if (a == op_to_grains[n])
                  {

                    aniso = eps_base;
                    anisox = 0.0;
                    anisoy = 0.0;
                    anisoz = 0.0;

                    anisoxplus = 0.0;
                    anisoyplus = 0.0;
                    anisozplus = 0.0;


                    TotGaussValue = 0.0;

                    // calculate the continuous anisotropy part 1

                    Val = (((*_vals[m])[_qp]) * ((*_vals[m])[_qp])) * (((*_vals[n])[_qp]) * ((*_vals[n])[_qp]));

                    // vector of gaussian exponent

                    ADReal vsmalbax = (*_grad_vals[m])[_qp](0) - (*_grad_vals[n])[_qp](0);
                    ADReal vsmalbay = (*_grad_vals[m])[_qp](1) - (*_grad_vals[n])[_qp](1);
                    ADReal vsmalbaz = (*_grad_vals[m])[_qp](2) - (*_grad_vals[n])[_qp](2);

                    ADReal normvsmallbavalue = (vsmalbax * vsmalbax) + (vsmalbay * vsmalbay) + (vsmalbaz * vsmalbaz);

                    ADReal normvsmallba = std::pow(normvsmallbavalue, .5);

                    ADReal vsmallbax = vsmalbax / normvsmallba;
                    ADReal vsmallbay = vsmalbay / normvsmallba;
                    ADReal vsmallbaz = vsmalbaz / normvsmallba;

                    _Vx[_qp] =  vsmallbax;
                    _Vy[_qp] =  vsmallbay;
                    _Vz[_qp] =  vsmallbaz;

                    if (sumval == 0.0)
                    {
                      vsmallbax == 0.0;
                      vsmallbay == 0.0;
                      vsmallbaz == 0.0;
                      _Vx[_qp] = 0.0;
                      _Vy[_qp] = 0.0;
                      _Vz[_qp] = 0.0;
                    }

                    // calculate quaternion qba components

                                  // Calculation method

                                  //qa = (a,b,c,d);   qb = (e,f,g,h);  q-1 = (x0, -x1, -x2, -x3);

                                  //qaqb[w] =  a*e  -  b*f  -  c*g  -  d*h
                                  //qaqb[x] =  a*f  +  b*e  +  c*h  -  d*g
                                  //qaqb[y] =  a*g  -  b*h  +  c*e  +  d*f
                                  //qaqb[z] =  a*h  +  b*g  -  c*f  +  d*e

                          //***************** qba = qa-1 * qb ******************************************************
                                  //a = qwR[a]
                                  //b = qxR[a] * (-1)
                                  //c = qyR[a] * (-1)
                                  //d = qzR[a] * (-1)

                                  //e = qwR[b]
                                  //f = qxR[b]
                                  //g = qyR[b]
                                  //h = qzR[b]

                    ADReal qbaw = ( qwR[a] * qwR[b] )   -   ( qxR[a] * (-1) * qxR[b] )   -   ( qyR[a] * (-1) * qyR[b] )   -   ( qzR[a] * (-1) * qzR[b] );
                    ADReal qbax = ( qwR[a] * qxR[b] )   +   ( qxR[a] * (-1) * qwR[b] )   +   ( qyR[a] * (-1) * qzR[b] )   -   ( qzR[a] * (-1) * qyR[b] );
                    ADReal qbay = ( qwR[a] * qyR[b] )   -   ( qxR[a] * (-1) * qzR[b] )   +   ( qyR[a] * (-1) * qwR[b] )   +   ( qzR[a] * (-1) * qxR[b] );
                    ADReal qbaz = ( qwR[a] * qzR[b] )   +   ( qxR[a] * (-1) * qyR[b] )   -   ( qyR[a] * (-1) * qxR[b] )   +   ( qzR[a] * (-1) * qwR[b] );


                   // convert anisotropy quaternion qba to vector vba and thetaba
                    ADReal thetaba = 2 * (std::acos(qbaw));

                    ADReal vbaxori = qbax / ( std::sin( (thetaba) / 2 ) ) ;
                    ADReal vbayori = qbay / ( std::sin( (thetaba) / 2 ) ) ;
                    ADReal vbazori = qbaz / ( std::sin( (thetaba) / 2 ) ) ;

                    ADReal rrrvalue = (vbaxori*vbaxori) + (vbayori*vbayori) + (vbazori*vbazori);

                    ADReal rrr = std::pow(rrrvalue, .5);

                    ADReal vbax = vbaxori/rrr;
                    ADReal vbay = vbayori/rrr;
                    ADReal vbaz = vbazori/rrr;


              ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                for (unsigned int l = 0; l < _libnum; ++l)
                {

                        ADReal miuvalue = (miubalibx[l] *miubalibx[l]) + (miubaliby[l] *miubaliby[l]) + (miubalibz[l] *miubalibz[l]);

                        ADReal mmm = std::pow(miuvalue , .5);

                        ADReal miubalibx_norm = miubalibx[l]/mmm;
                        ADReal miubaliby_norm = miubaliby[l]/mmm;
                        ADReal miubalibz_norm = miubalibz[l]/mmm;

                        ADReal miuow =  ( qwR[a] * (0.0) )            -   ( qxR[a] * miubalibx_norm )   -   ( qyR[a] * miubaliby_norm )   -   ( qzR[a] * miubalibz_norm ) ;
                        ADReal miuox =  ( qwR[a] * miubalibx_norm )   +   ( qxR[a] * (0.0) )            +   ( qyR[a] * miubalibz_norm )   -   ( qzR[a] * miubaliby_norm ) ;
                        ADReal miuoy =  ( qwR[a] * miubaliby_norm )   -   ( qxR[a] * miubalibz_norm )   +   ( qyR[a] * (0.0) )            +   ( qzR[a] * miubalibx_norm ) ;
                        ADReal miuoz =  ( qwR[a] * miubalibz_norm )   +   ( qxR[a] * miubaliby_norm )   -   ( qyR[a] * miubalibx_norm )   +   ( qzR[a] * (0.0) ) ;


                        // Determine whether the boundary normal is taken from the miinima library or compuuted from orientation quaterniionss
                        ADReal miubalibwR =  0.0;
                        ADReal miubalibxR =  miubalibx_norm;
                        ADReal miubalibyR =  miubaliby_norm;
                        ADReal miubalibzR =  miubalibz_norm;

                        if ( _BoundaryNormal == 0)
                        {
                            miubalibwR = ( (miuow) * (qwR[a])  )         -   ( (miuox) * (qxR[a]) * (-1) )   -   ( (miuoy) * (qyR[a]) * (-1) )   -   ( (miuoz) * (qzR[a]) * (-1) );
                            miubalibxR = ( (miuow) * (qxR[a]) * (-1) )   +   ( (miuox) * (qwR[a]) )          +   ( (miuoy) * (qzR[a]) * (-1) )   -   ( (miuoz) * (qyR[a]) * (-1) );
                            miubalibyR = ( (miuow) * (qyR[a]) * (-1) )   -   ( (miuox) * (qzR[a]) * (-1) )   +   ( (miuoy) * (qwR[a]) )          +   ( (miuoz) * (qxR[a]) * (-1) );
                            miubalibzR = ( (miuow) * (qzR[a]) * (-1) )   +   ( (miuox) * (qyR[a]) * (-1) )   -   ( (miuoy) * (qxR[a]) * (-1) )   +   ( (miuoz) * (qwR[a]) );
                        }


                        _VwlibR[_qp] =  miubalibwR;
                        _VxlibR[_qp] =  miubalibxR;
                        _VylibR[_qp] =  miubalibyR;
                        _VzlibR[_qp] =  miubalibzR;


                        if (sumval == 0.0)
                        {
                          miubalibwR == 0.0;
                          miubalibxR == 0.0;
                          miubalibyR == 0.0;
                          miubalibzR == 0.0;

                          _VwlibR[_qp] = 0.0;
                          _VxlibR[_qp] = 0.0;
                          _VylibR[_qp] = 0.0;
                          _VzlibR[_qp] = 0.0;
                        }


                        // dot product of normalized anisotropic vector and normalized gradient vector

                        ADReal dot_product = (miubalibxR* vsmallbax) + (miubalibyR * vsmallbay) + (miubalibzR * vsmallbaz);

                        // calculate the switch for a specific pair of grains and each library misorientation

                        if ((thetabalib[l]) < 0)
                        {
                          (thetabalib[l]) = 6.28319 + (thetabalib[l]);
                        }

                        ADReal thetadtheta = thetaba - (thetabalib[l]);

                        ADReal normvbavalue = (vbax * vbax) + (vbay * vbay) + (vbaz * vbaz);

                        ADReal normvba = std::pow(normvbavalue, .5);

                        ADReal normvbalibvalue = ( vbalibx[l] * vbalibx[l] ) + ( vbaliby[l] * vbaliby[l] ) + ( vbalibz[l] * vbalibz[l] );

                        ADReal normvbalib = std::pow(normvbalibvalue, .5);

                        ADReal cosinevalue1 = ( (vbalibx[l]) * vbax ) + ( (vbaliby[l]) * vbay ) + ( (vbalibz[l]) * vbaz );

                        ADReal cosinevalue2 = normvbalib * normvba;

                        ADReal cosinevalue3 = cosinevalue1 / cosinevalue2;

                        ADReal thetadv = std::acos(cosinevalue3);

                        ADReal switchvalue = (-100) * (((thetadv / _alphaswitch) * (thetadv / _alphaswitch)) + ((thetadtheta / _betaswitch) * (thetadtheta / _betaswitch)));

                        _S_switch[_qp] = std::exp(switchvalue);

                        if (_S_switch[_qp] < 0.001)
                        {
                          _S_switch[_qp] = 0.0;
                        }


                        //calculate the gaussian anisotropy with Derivative of anisotropy
                        ADReal exponent = _sharpness * (dot_product - 1.0);

                        ADReal amplitudes = ( (_sigmaBASE - (MinimaEnergy[l]))* (_lgb / 1.0e9) *(std::sqrt(f0saddle)) ) / g_gamma;


                        ADReal tol = _Gaussian_Tolerance;
                        if(abs(normvsmallba) > tol) // if R < tol
                          {


                              ADReal Gaussian = _JtoeV * _length_scale *  _amplitudeScale * amplitudes * _S_switch[_qp] * std::exp(exponent);
                              Gaussian = _ADDGaussian ? Gaussian : -Gaussian;  // yes = add, false = subtract


                              ADReal Gaussian2 = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((-1) * miubalibxR * normvsmallba ) - (miubalibxR * vsmalbax * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbax)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbax))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbax))) / normvsmallbavalue) );
                                                Gaussian2 = _ADDGaussian ? Gaussian2 : -Gaussian2;  // yes = add, false = subtract

                              ADReal Gaussian3 = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((-1) * miubalibyR * normvsmallba ) - (miubalibyR * vsmalbay * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbay)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbay))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbay))) / normvsmallbavalue) );
                                                Gaussian3 = _ADDGaussian ? Gaussian3 : -Gaussian3;  // yes = add, false = subtract

                              ADReal Gaussian4 = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((-1) * miubalibzR * normvsmallba ) - (miubalibzR * vsmalbaz * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbaz)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbaz))) / normvsmallbavalue) + ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbaz))) / normvsmallbavalue) );
                                                Gaussian4 = _ADDGaussian ? Gaussian4 : -Gaussian4;  // yes = add, false = subtract


                              ADReal Gaussian2plus = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((1) * miubalibxR * normvsmallba ) - (miubalibxR * vsmalbax * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbax)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbax))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbax))) / normvsmallbavalue) );
                                                Gaussian2plus = _ADDGaussian ? Gaussian2plus : -Gaussian2plus;  // yes = add, false = subtract
                              ADReal Gaussian3plus = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((1) * miubalibyR * normvsmallba ) - (miubalibyR * vsmalbay * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbay)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbay))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbay))) / normvsmallbavalue) );
                                                Gaussian3plus = _ADDGaussian ? Gaussian3plus : -Gaussian3plus;  // yes = add, false = subtract
                              ADReal Gaussian4plus = _JtoeV * _length_scale * _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) * ( ( ( ((1) * miubalibzR * normvsmallba ) - (miubalibzR * vsmalbaz * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbaz)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbaz))) / normvsmallbavalue) + ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbaz))) / normvsmallbavalue) );
                                                Gaussian4plus = _ADDGaussian ? Gaussian4plus : -Gaussian4plus;  // yes = add, false = subtract



                              TotGaussValue += Gaussian;

                              aniso += Gaussian;
                              anisox += Gaussian2;
                              anisoy += Gaussian3;
                              anisoz += Gaussian4;

                              anisoxplus += Gaussian2plus;
                              anisoyplus += Gaussian3plus;
                              anisozplus += Gaussian4plus;

                          } // R-loop

                      eps_min = ( (MinimaEnergy[l] *_JtoeV * _length_scale * _length_scale) * _lgb *  (std::sqrt(f0saddle)) ) / (g_gamma) ;


                    } // l - linmun loop

                    sumeps += aniso * Val;
                    sumepsx += anisox * Val;
                    sumepsy += anisoy * Val;
                    sumepsz += anisoz * Val;

                    sumepsxplus += anisoxplus * Val;
                    sumepsyplus += anisoyplus * Val;
                    sumepszplus += anisozplus * Val;


                    SumTotGaussValue +=  TotGaussValue * Val;

                    Totepsbar += eps_base * Val;
                    Totepsmin += eps_min * Val;

                  } // if a == op_to_grains[n]

                } // if b == op_to_grains[m]

              } // n - op_num loop

            } // a - grain_num loop

          } // m - op_num loop

        } // b - grain_num loop


        //************************************* Caluculation anisotropy and the derivatives *****************************************
        _eps[_qp] = abs(sumeps / sumval) ;
        _epsenergy[_qp] = MetaPhysicL::raw_value(sumeps / sumval);
        _depsdx[_qp] = sumepsx / sumval;
        _depsdy[_qp] = sumepsy / sumval;
        _depsdz[_qp] = sumepsz / sumval;

        _depsdx_EN[_qp] = MetaPhysicL::raw_value(sumepsx / sumval);
        _depsdy_EN[_qp] = MetaPhysicL::raw_value(sumepsy / sumval);
        _depsdz_EN[_qp] = MetaPhysicL::raw_value(sumepsz / sumval);

        _depsdxplus[_qp] = sumepsxplus / sumval;
        _depsdyplus[_qp] = sumepsyplus / sumval;
        _depsdzplus[_qp] = sumepszplus / sumval;
        _depsdxplus_EN[_qp] = MetaPhysicL::raw_value(sumepsxplus / sumval);
        _depsdyplus_EN[_qp] = MetaPhysicL::raw_value(sumepsyplus / sumval);
        _depsdzplus_EN[_qp] = MetaPhysicL::raw_value(sumepszplus / sumval);

        _epsbar[_qp] = Totepsbar / sumval;
        _epsmin2grains[_qp] = Totepsmin / sumval;
        _TotGauss[_qp] = SumTotGaussValue / sumval;

        if (sumval == 0.0)
          {
            _eps[_qp] = eps_base;
            _epsenergy[_qp] = MetaPhysicL::raw_value((3.0/4.0) * (_sigmaBASE *_JtoeV * _length_scale * _length_scale) * _lgb);

            _depsdx[_qp] = 0.0;
            _depsdy[_qp] = 0.0;
            _depsdz[_qp] =  0.0;

            _depsdx_EN[_qp] = 0.0;
            _depsdy_EN[_qp] = 0.0;
            _depsdz_EN[_qp] = 0.0;

            _depsdxplus[_qp] = 0.0;
            _depsdyplus[_qp] = 0.0;
            _depsdzplus[_qp] =  0.0;
            _depsdxplus_EN[_qp] = 0.0;
            _depsdyplus_EN[_qp] = 0.0;
            _depsdzplus_EN[_qp] = 0.0;

            _epsbar[_qp] = 0.0;
            _epsmin2grains[_qp] = 0.0;
            _TotGauss[_qp] = 0.0;
          }
        else  if (_eps[_qp] == 0.0)
          {
            _eps[_qp] = eps_base;
            _epsenergy[_qp] = MetaPhysicL::raw_value((3.0/4.0) * (_sigmaBASE *_JtoeV * _length_scale * _length_scale) * _lgb);

            _depsdx[_qp] = 0.0;
            _depsdy[_qp] = 0.0;
            _depsdz[_qp] =  0.0;

            _depsdx_EN[_qp] = 0.0;
            _depsdy_EN[_qp] = 0.0;
            _depsdz_EN[_qp] = 0.0;

            _depsdxplus[_qp] = 0.0;
            _depsdyplus[_qp] = 0.0;
            _depsdzplus[_qp] =  0.0;
            _depsdxplus_EN[_qp] = 0.0;
            _depsdyplus_EN[_qp] = 0.0;
            _depsdzplus_EN[_qp] = 0.0;

            _epsbar[_qp] = 0.0;
            _epsmin2grains[_qp] = 0.0;
            _TotGauss[_qp] = 0.0;
          }

       //*************************************** Caluculation for m and sigma *****************************************
        ADReal m_value = ((((_eps[_qp])*1))/ (_lgb * _lgb)) / f0saddle;

        _m[_qp] = m_value;

        _m_energy[_qp] = MetaPhysicL::raw_value(m_value);

        ADReal con = 1 / ( (_lgb * _lgb) * f0saddle);
        _dmdx[_qp] = con * _depsdx[_qp] ;
        _dmdy[_qp] = con * _depsdy[_qp] ;
        _dmdz[_qp] = con * _depsdz[_qp] ;
        _dmdx_EN[_qp] = MetaPhysicL::raw_value(con * _depsdx_EN[_qp] );
        _dmdy_EN[_qp] = MetaPhysicL::raw_value(con * _depsdy_EN[_qp] );
        _dmdz_EN[_qp] = MetaPhysicL::raw_value(con * _depsdz_EN[_qp] );

        _dmdxplus[_qp] = con * _depsdxplus[_qp] ;
        _dmdyplus[_qp] = con * _depsdyplus[_qp] ;
        _dmdzplus[_qp] = con * _depsdzplus[_qp] ;
        _dmdxplus_EN[_qp] = MetaPhysicL::raw_value(con * _depsdxplus_EN[_qp] );
        _dmdyplus_EN[_qp] = MetaPhysicL::raw_value(con * _depsdyplus_EN[_qp] );
        _dmdzplus_EN[_qp] = MetaPhysicL::raw_value(con * _depsdzplus_EN[_qp] );


        ADReal value = (_eps[_qp]) * (_m[_qp]);

        ADReal sigma_value = g_gamma * ( std::sqrt( value ) );

        _sigma[_qp] = sigma_value;
        _sigmaORIUNIT[_qp] = sigma_value/(_JtoeV * _length_scale * _length_scale);


        _epsCalc[_qp] = (3.0/4.0) * (_sigma[_qp]) * _lgb;
        _mCalc[_qp] = (3.0/4.0) * ( (_sigma[_qp])/ (f0saddle * _lgb) );



        //************************************** Caluculation for L *****************************************
        ADReal length_scale4 = _length_scale * _length_scale * _length_scale * _length_scale;

        ADReal Mgb = 0;

        if ( _Q > 0)
        {
          // Convert to lengthscale^4/(eV*timescale);
          ADReal M0gb = _Mob * _time_scale / (_JtoeV * length_scale4);

                 Mgb = M0gb * std::exp(-_Q / (_kb * _T));
        }
        else
        {
          // Convert to lengthscale^4/(eV*timescale)
                 Mgb = _GBMobility * _time_scale / (_JtoeV * length_scale4);
        }


        _L[_qp] = (4.0 / 3.0) * (Mgb /_lgb);

        _L_energy[_qp] = MetaPhysicL::raw_value ((4.0 / 3.0) * (Mgb /_lgb) );

        //**********************************************************************************************************

  } // void bracket
