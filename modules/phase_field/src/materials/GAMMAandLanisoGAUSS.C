
// 5D Gaussian anisotropy material object
// Material properties adding anysotroopy to γ and L.


#include "GAMMAandLanisoGAUSS.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "GrainTrackerInterface.h"

#include <fstream>
#include <iostream>
#include <vector>


registerMooseObject("PhaseFieldApp", GAMMAandLanisoGAUSS);

InputParameters
GAMMAandLanisoGAUSS::validParams()
{
  InputParameters params = ADMaterial::validParams();

  params.addClassDescription("Material to provide an anisotropy field and its derivatives wrt to gradient of order parameter");

  params.addRequiredParam<MaterialPropertyName>("Ggamma_name", "The name of Ggamma");
      params.addRequiredParam<MaterialPropertyName>("Ggamma_EN_name", "The name of Ggamma_EN");

  params.addRequiredParam<MaterialPropertyName>("dGgammadx_name", "The name of dGgammadx, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammadx_EN_name", "The name of dGgammadx_EN, WRT grad eta");

  params.addRequiredParam<MaterialPropertyName>("dGgammady_name", "The name of dGgammady, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammady_EN_name", "The name of dGgammady_EN, WRT grad eta");

  params.addRequiredParam<MaterialPropertyName>("dGgammadz_name", "The name of dGgammadz, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammadz_EN_name", "The name of dGgammadz_EN, WRT grad eta");


  params.addRequiredParam<MaterialPropertyName>("dGgammadxplus_name", "The name of dGgammadx, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammadxplus_EN_name", "The name of dGgammadx_EN, WRT grad eta");
  params.addRequiredParam<MaterialPropertyName>("dGgammadyplus_name", "The name of dGgammady, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammadyplus_EN_name", "The name of dGgammady_EN, WRT grad eta");
  params.addRequiredParam<MaterialPropertyName>("dGgammadzplus_name", "The name of dGgammadz, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dGgammadzplus_EN_name", "The name of dGgammadz_EN, WRT grad eta");


  params.addRequiredParam<MaterialPropertyName>("gamma_name", "The name of gamma");
      params.addRequiredParam<MaterialPropertyName>("gamma_EN_name", "The name of gamma_EN");

  params.addRequiredParam<MaterialPropertyName>("S_switch_name", "The name of switch");

  params.addRequiredParam<MaterialPropertyName>("dgammadx_name", "The name of dgammadx, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammadx_EN_name", "The name of dgammadx_EN, WRT grad eta");

  params.addRequiredParam<MaterialPropertyName>("dgammady_name", "The name of dgammady, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammady_EN_name", "The name of dgammady_EN, WRT grad eta");

  params.addRequiredParam<MaterialPropertyName>("dgammadz_name", "The name of dgammadz, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammadz_EN_name", "The name of dgammadz_EN, WRT grad eta");


  params.addRequiredParam<MaterialPropertyName>("dgammadxplus_name", "The name of dgammadx, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammadxplus_EN_name", "The name of dgammadx_EN, WRT grad eta");
  params.addRequiredParam<MaterialPropertyName>("dgammadyplus_name", "The name of dgammady, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammadyplus_EN_name", "The name of dgammady_EN, WRT grad eta");
  params.addRequiredParam<MaterialPropertyName>("dgammadzplus_name", "The name of dgammadz, WRT grad eta");
      params.addRequiredParam<MaterialPropertyName>("dgammadzplus_EN_name", "The name of dgammadz_EN, WRT grad eta");

  params.addRequiredParam<MaterialPropertyName>("VwlibR_name", "The name of the library boundary normal in simulation Reference frame R");
  params.addRequiredParam<MaterialPropertyName>("VxlibR_name", "The name of the library boundary normal in simulation Reference frame R in x direction");
  params.addRequiredParam<MaterialPropertyName>("VylibR_name", "The name of the library boundary normal in simulation Reference frame R in y direction");
  params.addRequiredParam<MaterialPropertyName>("VzlibR_name", "The name of the library boundary normal in simulation Reference frame R in z direction");
  params.addRequiredParam<MaterialPropertyName>("Vx_name", "The name of normmaliized gradiient in x directtion");
  params.addRequiredParam<MaterialPropertyName>("Vy_name", "The name of normmaliized gradiient in y directtion");
  params.addRequiredParam<MaterialPropertyName>("Vz_name", "The name of normmaliized gradiient in z directtion");


  params.addRequiredParam<MaterialPropertyName>("m_name", "The name of m");
      params.addRequiredParam<MaterialPropertyName>("m_EN_name", "The name of m_EN");

  params.addRequiredParam<MaterialPropertyName>("kappa_name", "The name of kappa");
      params.addRequiredParam<MaterialPropertyName>("kappa_EN_name", "The name of kappa_EN");

  params.addRequiredParam<MaterialPropertyName>("L_name", "The name of L");
      params.addRequiredParam<MaterialPropertyName>("L_EN_name", "The name of L_EN");

  params.addRequiredParam<MaterialPropertyName>("lgb_name", "The name of lgb");
      params.addRequiredParam<MaterialPropertyName>("lgb_EN_name", "The name of lgb_EN");

  params.addRequiredParam<MaterialPropertyName>("sigma_name", "Grain boundary energy, in eV/nm^2");
      params.addRequiredParam<MaterialPropertyName>("sigma_EN_name", "Grain boundary energy, in eV/nm^2");

  params.addRequiredParam<MaterialPropertyName>("sigmaORIUNIT_name", "Grain boundary energy, in J/m^2");
      params.addRequiredParam<MaterialPropertyName>("sigmaORIUNIT_EN_name", "Grain boundary energy, in J/m^2");

  params.addRequiredParam<MaterialPropertyName>("qwg_name", "w component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qxg_name", "x component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qyg_name", "y component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("qzg_name", "z component of quaternion");

  params.addRequiredParam<MaterialPropertyName>("Ggammabar_name", "The name of the average anisotropy");

  params.addRequiredParam<MaterialPropertyName>("Ggammamin2grains_name", "The name of the 2grains minimum Ggamma");

  params.addRequiredParam<MaterialPropertyName>("TotGauss_name", "The total gaussian to add/remove");

  params.addRequiredParam<MaterialPropertyName>("MGBVALUE_name", "The value of MGB");

  params.addRequiredParam<FileName>("Library_file_name",  "Name of the file containing library quaternion data");

  params.addRequiredParam<FileName>("Quaternion_file_name",  "Name of the file containing anisotropy data");

  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");

  params.addRequiredParam<Real>("sigmaBASE", "value of sigmaBASE energy, maximum energy");

  params.addRequiredParam<Real>("gammaBASE", "value of gamma at sigmaBASE energy");

  params.addRequiredParam<Real>("GgammaBASE", "value of Ggamma at sigmaBASE energy");

  params.addRequiredParam<Real>("f0gammaBASE", "value of f0gamma at sigmaBASE energy");

  params.addRequiredParam<Real>("L_BASE", "value of L_BASE");

  params.addRequiredParam<Real>("lgbBASE_minimum", "value of lgbBASE_minimum");

  params.addRequiredParam<Real>("alphaswitch", "acceptable range of angle from vbalib");

  params.addRequiredParam<Real>("betaswitch", "acceptable range of angle from thetabalib");

  params.addRequiredParam<int>("libnum", "number of misorientations in the library");

  params.addRequiredParam<Real>("amplitudeScale", "amplitudeScale of gaussians");

  params.addRequiredParam<Real>("sharpness", "sharpness of gaussians");

  params.addRequiredParam<Real>("Gaussian_Tolerance", "Gaussian Tolerance");

  params.addRequiredParam<UserObjectName>("grain_tracker","graintracker UserObject");

  params.addRequiredParam<bool>("ADDGaussian", "If this is true gaussians add to gammaBASE, else subtract");

  params.addRequiredParam<bool>("ADDGaussianL", "If this is true gaussians add to L_BASE, else subtract");

  params.addParam<Real>("length_scale", 1.0e-9, "conversion for 1/m to 1/nm");

  params.addParam<Real>("time_scale", 1.0e-9, "Time scale in s, where default is ns");

  params.addParam<Real>("JtoeV", 6.24150974e18, "Joule to eV conversions");

  params.addParam<Real>("BoundaryNormal", 0, "if = 0,the boundary normal in the simulation reference frame is computed using the orientations quaternions, else it is taken directly from the minima library file");


  return params;
}

GAMMAandLanisoGAUSS::GAMMAandLanisoGAUSS(const InputParameters & parameters)
  : ADMaterial(parameters),

    _Ggamma(declareADProperty<Real>(getParam<MaterialPropertyName>("Ggamma_name"))),
        _Ggamma_EN(declareProperty<Real>(getParam<MaterialPropertyName>("Ggamma_EN_name"))),

    _dGgammadx(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammadx_name"))),
        _dGgammadx_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammadx_EN_name"))),
    _dGgammady(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammady_name"))),
        _dGgammady_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammady_EN_name"))),
    _dGgammadz(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammadz_name"))),
        _dGgammadz_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammadz_EN_name"))),


    _dGgammadxplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammadxplus_name"))),
        _dGgammadxplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammadxplus_EN_name"))),
    _dGgammadyplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammadyplus_name"))),
        _dGgammadyplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammadyplus_EN_name"))),
    _dGgammadzplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dGgammadzplus_name"))),
        _dGgammadzplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dGgammadzplus_EN_name"))),

    _VwlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VwlibR_name"))),
    _VxlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VxlibR_name"))),
    _VylibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VylibR_name"))),
    _VzlibR(declareADProperty<Real>(getParam<MaterialPropertyName>("VzlibR_name"))),

    _Vx(declareADProperty<Real>(getParam<MaterialPropertyName>("Vx_name"))),
    _Vy(declareADProperty<Real>(getParam<MaterialPropertyName>("Vy_name"))),
    _Vz(declareADProperty<Real>(getParam<MaterialPropertyName>("Vz_name"))),

    _gamma(declareADProperty<Real>(getParam<MaterialPropertyName>("gamma_name"))),
        _gamma_EN(declareProperty<Real>(getParam<MaterialPropertyName>("gamma_EN_name"))),

    _S_switch(declareADProperty<Real>(getParam<MaterialPropertyName>("S_switch_name"))),

    _dgammadx(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammadx_name"))),
        _dgammadx_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammadx_EN_name"))),
    _dgammady(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammady_name"))),
        _dgammady_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammady_EN_name"))),
    _dgammadz(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammadz_name"))),
        _dgammadz_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammadz_EN_name"))),

    _dgammadxplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammadxplus_name"))),
        _dgammadxplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammadxplus_EN_name"))),
    _dgammadyplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammadyplus_name"))),
        _dgammadyplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammadyplus_EN_name"))),
    _dgammadzplus(declareADProperty<Real>(getParam<MaterialPropertyName>("dgammadzplus_name"))),
        _dgammadzplus_EN(declareProperty<Real>(getParam<MaterialPropertyName>("dgammadzplus_EN_name"))),

    _m(declareADProperty<Real>(getParam<MaterialPropertyName>("m_name"))),
        _m_EN(declareProperty<Real>(getParam<MaterialPropertyName>("m_EN_name"))),

    _kappa(declareADProperty<Real>(getParam<MaterialPropertyName>("kappa_name"))),
        _kappa_EN(declareProperty<Real>(getParam<MaterialPropertyName>("kappa_EN_name"))),

    _L(declareADProperty<Real>(getParam<MaterialPropertyName>("L_name"))),
        _L_EN(declareProperty<Real>(getParam<MaterialPropertyName>("L_EN_name"))),

    _lgb(declareADProperty<Real>(getParam<MaterialPropertyName>("lgb_name"))),
        _lgb_EN(declareProperty<Real>(getParam<MaterialPropertyName>("lgb_EN_name"))),

    _ADDGaussian(getParam<bool>("ADDGaussian")),

    _ADDGaussianL(getParam<bool>("ADDGaussianL")),

    _sigma(declareADProperty<Real>(getParam<MaterialPropertyName>("sigma_name"))),
        _sigma_EN(declareProperty<Real>(getParam<MaterialPropertyName>("sigma_EN_name"))),

    _sigmaORIUNIT(declareADProperty<Real>(getParam<MaterialPropertyName>("sigmaORIUNIT_name"))),
        _sigmaORIUNIT_EN(declareProperty<Real>(getParam<MaterialPropertyName>("sigmaORIUNIT_EN_name"))),

    _qwg(declareADProperty<Real>(getParam<MaterialPropertyName>("qwg_name"))),

    _qxg(declareADProperty<Real>(getParam<MaterialPropertyName>("qxg_name"))),

    _qyg(declareADProperty<Real>(getParam<MaterialPropertyName>("qyg_name"))),

    _qzg(declareADProperty<Real>(getParam<MaterialPropertyName>("qzg_name"))),

    _Ggammabar(declareADProperty<Real>(getParam<MaterialPropertyName>("Ggammabar_name"))),

    _Ggammamin2grains(declareADProperty<Real>(getParam<MaterialPropertyName>("Ggammamin2grains_name"))),

    _TotGauss(declareADProperty<Real>(getParam<MaterialPropertyName>("TotGauss_name"))),

    _MGBVALUE(declareADProperty<Real>(getParam<MaterialPropertyName>("MGBVALUE_name"))),

    _sigmaBASE(getParam<Real>("sigmaBASE")),

    _GgammaBASE(getParam<Real>("GgammaBASE")),

    _gammaBASE(getParam<Real>("gammaBASE")),

    _f0gammaBASE(getParam<Real>("f0gammaBASE")),

    _L_BASE(getParam<Real>("L_BASE")),

    _lgbBASE_minimum(getParam<Real>("lgbBASE_minimum")),

    _alphaswitch(getParam<Real>("alphaswitch")),

    _betaswitch(getParam<Real>("betaswitch")),

    _libnum(getParam<int>("libnum")),

    _amplitudeScale(getParam<Real>("amplitudeScale")),

    _sharpness(getParam<Real>("sharpness")),

    _Gaussian_Tolerance(getParam<Real>("Gaussian_Tolerance")),

    _Library_file_name(getParam<FileName>("Library_file_name")),

    _Quaternion_file_name(getParam<FileName>("Quaternion_file_name")),

    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")),

    _length_scale(getParam<Real>("length_scale")),

    _time_scale(getParam<Real>("time_scale")),

    _JtoeV(getParam<Real>("JtoeV")),

    _BoundaryNormal(getParam<Real>("BoundaryNormal")),

    _op_num(coupledComponents("v")),

    _vals(adCoupledValues("v")),

    _grad_vals(adCoupledGradients("v"))



//##########################################################################################################################


    {
      if (_op_num == 0)
        paramError("op_num", "op_num must be greater than 0");

    }
//#########################################################################################################################


void
GAMMAandLanisoGAUSS::computeQpProperties()
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
        std::vector<double> MinimaL;

        double Lib1;
        double Lib2;
        double Lib3;
        double Lib4;
        double Lib5;
        double Lib6;
        double Lib7;
        double Lib8;
        double Lib9;

        while (!File2.eof())
        {
          File2 >> Lib1 >> Lib2 >> Lib3 >> Lib4 >> Lib5 >> Lib6 >> Lib7 >> Lib8 >> Lib9;
          vbalibx.push_back(Lib1);
          vbaliby.push_back(Lib2);
          vbalibz.push_back(Lib3);
          thetabalib.push_back(Lib4);
          miubalibx.push_back(Lib5);
          miubaliby.push_back(Lib6);
          miubalibz.push_back(Lib7);
          MinimaEnergy.push_back(Lib8);
          MinimaL.push_back(Lib9);
        }

        File2.close();


        //******************************* Grain quaternion outputs ****************

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

        ADReal sigmaBASE =  _sigmaBASE * _JtoeV * _length_scale * _length_scale;

        _m[_qp] = std::sqrt((sigmaBASE * sigmaBASE)/ ( (_lgbBASE_minimum * _lgbBASE_minimum) * _f0gammaBASE * (_GgammaBASE * _GgammaBASE) ));
          _m_EN[_qp] = MetaPhysicL::raw_value( std::sqrt((sigmaBASE * sigmaBASE) / ( (_lgbBASE_minimum * _lgbBASE_minimum) * _f0gammaBASE * (_GgammaBASE * _GgammaBASE) )) );

        _kappa[_qp] = (sigmaBASE * sigmaBASE) / ( (_GgammaBASE * _GgammaBASE) * _m[_qp] );
          _kappa_EN[_qp] = MetaPhysicL::raw_value( (sigmaBASE*sigmaBASE) / ( (_GgammaBASE*_GgammaBASE) * _m[_qp] ) );



        // ADReal S_switch = 0.0;
        ADReal sumGgamma = 0.0;
        ADReal sumGgammax = 0.0;
        ADReal sumGgammay = 0.0;
        ADReal sumGgammaz = 0.0;

        ADReal sumGgammaxplus = 0.0;
        ADReal sumGgammayplus = 0.0;
        ADReal sumGgammazplus = 0.0;

        ADReal sumL = 0.0;

        ADReal aniso = 0.0;
        ADReal anisox = 0.0;
        ADReal anisoy = 0.0;
        ADReal anisoz = 0.0;

        ADReal anisoL  = 0.0;

        ADReal anisoxplus = 0.0;
        ADReal anisoyplus = 0.0;
        ADReal anisozplus = 0.0;


        ADReal Val = 0.0;
        ADReal Val1 = 0.0;
        ADReal sumval = 0.0;
        ADReal TotGaussValue = 0.0;
        ADReal SumTotGaussValue = 0.0;
        ADReal TotGgammabar = 0.0;
        ADReal TotGgammamin = 0.0;
        ADReal Ggamma_min = 0.0;

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

                    aniso = _GgammaBASE;
                    anisox = 0.0;
                    anisoy = 0.0;
                    anisoz = 0.0;

                    anisoxplus = 0.0;
                    anisoyplus = 0.0;
                    anisozplus = 0.0;

                    anisoL = _L_BASE;

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

                        if (_S_switch[_qp] < 0.0001)
                        {
                          _S_switch[_qp] = 0.0;
                        }

                        //calculate amplitudes and lgb_Minima

                        ADReal Ggamma_Minima = (MinimaEnergy[l] * _JtoeV * _length_scale * _length_scale) / ( std::sqrt(_kappa[_qp] * _m[_qp]) );


                        ADReal amplitudes = _GgammaBASE - Ggamma_Minima;

                        ADReal amplitudesL = abs( _L_BASE - MinimaL[l]);

                        ADReal g2_Minima = Ggamma_Minima * Ggamma_Minima;

                        ADReal p_Minima = -(3.0944*(g2_Minima*g2_Minima*g2_Minima*g2_Minima)) -(1.8169*(g2_Minima*g2_Minima*g2_Minima)) + (10.323*(g2_Minima*g2_Minima)) - (8.1819*(g2_Minima)) + 2.0033;

                        ADReal gamma_Minima = 1/p_Minima;

                        ADReal Inversegamma_Minima = 1/gamma_Minima;
                        ADReal Inversegamma_Minima1 = Inversegamma_Minima;
                        ADReal Inversegamma_Minima2 = Inversegamma_Minima * Inversegamma_Minima;
                        ADReal Inversegamma_Minima3 = Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima;
                        ADReal Inversegamma_Minima4 = Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima;
                        ADReal Inversegamma_Minima5 = Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima;
                        ADReal Inversegamma_Minima6 = Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima * Inversegamma_Minima;

                        ADReal f0gamma_Minima = (0.0788 * Inversegamma_Minima6) - (0.4955 * Inversegamma_Minima5) + (1.2244 * Inversegamma_Minima4) - (1.5281 * Inversegamma_Minima3) + (1.0686 * Inversegamma_Minima2) - (0.5563 * Inversegamma_Minima1) + 0.2907;

                        ADReal lgb_Minima = std::sqrt( (_kappa[_qp] / (_m[_qp] * f0gamma_Minima)) );

                        //calculate the gaussian anisotropy with Derivative of anisotropy

                        ADReal exponent = _sharpness * (dot_product - 1.0);


                ADReal tol = _Gaussian_Tolerance;
                if(abs(normvsmallba) > tol) // if R < tol, no contributions from gaussians 1e-20
                  {

                        ADReal Gaussian =  _amplitudeScale * amplitudes * _S_switch[_qp] * std::exp(exponent);
                        Gaussian = _ADDGaussian ? Gaussian : -Gaussian;  // yes = add, false = subtract

                        ADReal GaussianL =    amplitudesL * _S_switch[_qp] * std::exp(exponent);
                        GaussianL = _ADDGaussianL ? GaussianL : -GaussianL;  // yes = add, false = subtract


                        ADReal Gaussian2 = _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((-1) * miubalibxR * normvsmallba ) - (miubalibxR * vsmalbax * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbax)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbax))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbax))) / normvsmallbavalue) );
                        Gaussian2 = _ADDGaussian ? Gaussian2 : -Gaussian2;  // yes = add, false = subtract

                        ADReal Gaussian3 =  _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((-1) * miubalibyR * normvsmallba ) - (miubalibyR * vsmalbay * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbay)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbay))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbay))) / normvsmallbavalue) );
                        Gaussian3 = _ADDGaussian ? Gaussian3 : -Gaussian3;  // yes = add, false = subtract

                        ADReal Gaussian4 = _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((-1) * miubalibzR * normvsmallba ) - (miubalibzR * vsmalbaz * (1 / (2 * normvsmallba)) * ( (-1) * 2 * vsmalbaz)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbaz))) / normvsmallbavalue) + ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (-1) * vsmalbaz))) / normvsmallbavalue) );
                        Gaussian4 = _ADDGaussian ? Gaussian4 : -Gaussian4;  // yes = add, false = subtract


                        ADReal Gaussian2plus = _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((1) * miubalibxR * normvsmallba ) - (miubalibxR * vsmalbax * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbax)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbax))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbax))) / normvsmallbavalue) );
                        Gaussian2plus = _ADDGaussian ? Gaussian2plus : -Gaussian2plus;  // yes = add, false = subtract

                        ADReal Gaussian3plus =  _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((1) * miubalibyR * normvsmallba ) - (miubalibyR * vsmalbay * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbay)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbay))) / normvsmallbavalue) + ( ( (-1) * (miubalibzR * vsmalbaz) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbay))) / normvsmallbavalue) );
                        Gaussian3plus = _ADDGaussian ? Gaussian3plus : -Gaussian3plus;  // yes = add, false = subtract

                        ADReal Gaussian4plus = _amplitudeScale * amplitudes  * _S_switch[_qp] * _sharpness * std::exp(exponent) *   ( ( ( ((1) * miubalibzR * normvsmallba ) - (miubalibzR * vsmalbaz * (1 / (2 * normvsmallba)) * ( (1) * 2 * vsmalbaz)) ) / normvsmallbavalue ) +  ( ( (-1) * (miubalibxR * vsmalbax) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbaz))) / normvsmallbavalue) + ( ( (-1) * (miubalibyR * vsmalbay) * ( (1 / (2 * normvsmallba)) * (2 * (1) * vsmalbaz))) / normvsmallbavalue) );
                        Gaussian4plus = _ADDGaussian ? Gaussian4plus : -Gaussian4plus;  // yes = add, false = subtract


                        TotGaussValue += Gaussian;

                        aniso += Gaussian;
                        anisox += Gaussian2;
                        anisoy += Gaussian3;
                        anisoz += Gaussian4;

                        anisoL += GaussianL;

                        anisoxplus += Gaussian2plus;
                        anisoyplus += Gaussian3plus;
                        anisozplus += Gaussian4plus;


                    } // R-loop

                Ggamma_min = Ggamma_Minima ;

                } // l - linmun loop

                    sumGgamma += aniso * Val;
                    sumGgammax += anisox * Val;
                    sumGgammay += anisoy * Val;
                    sumGgammaz += anisoz * Val;

                    sumGgammaxplus += anisoxplus * Val;
                    sumGgammayplus += anisoyplus * Val;
                    sumGgammazplus += anisozplus * Val;

                    sumL += anisoL * Val;

                    SumTotGaussValue +=  TotGaussValue * Val;

                    TotGgammabar += _GgammaBASE * Val;
                    TotGgammamin += Ggamma_min * Val;


                  } // if a == op_to_grains[n]

                } // if b == op_to_grains[m]

              } // n - op_num loop

            } // a - grain_num loop

          } // m - op_num loop

        } // b - grain_num loop


        //************************************* Caluculation  of Ggamma, L and the derivatives **s***************************************
        _L[_qp] = abs(sumL / sumval);
        _L_EN[_qp] = MetaPhysicL::raw_value(sumL / sumval);

        _Ggamma[_qp] = abs(sumGgamma / sumval);
          _Ggamma_EN[_qp] = MetaPhysicL::raw_value(sumGgamma / sumval);

        _dGgammadx[_qp] = sumGgammax / sumval;
        _dGgammady[_qp] = sumGgammay / sumval;
        _dGgammadz[_qp] =  sumGgammaz / sumval;
          _dGgammadx_EN[_qp] = MetaPhysicL::raw_value(sumGgammax / sumval);
          _dGgammady_EN[_qp] = MetaPhysicL::raw_value(sumGgammay / sumval);
          _dGgammadz_EN[_qp] = MetaPhysicL::raw_value(sumGgammaz / sumval);


       _dGgammadxplus[_qp] = sumGgammaxplus / sumval;
       _dGgammadyplus[_qp] = sumGgammayplus / sumval;
       _dGgammadzplus[_qp] =  sumGgammazplus / sumval;
         _dGgammadxplus_EN[_qp] = MetaPhysicL::raw_value(sumGgammaxplus / sumval);
         _dGgammadyplus_EN[_qp] = MetaPhysicL::raw_value(sumGgammayplus / sumval);
         _dGgammadzplus_EN[_qp] = MetaPhysicL::raw_value(sumGgammazplus / sumval);


        _Ggammabar[_qp] = TotGgammabar / sumval;
        _Ggammamin2grains[_qp] = TotGgammamin / sumval;
        _TotGauss[_qp] = SumTotGaussValue / sumval;


        if (sumval == 0.00)
          {
            _L[_qp] = _L_BASE;
            _L_EN[_qp] = MetaPhysicL::raw_value(_L_BASE * 1);

            _Ggamma[_qp] = _GgammaBASE;
              _Ggamma_EN[_qp] = _GgammaBASE;

            _dGgammadx[_qp] = 0.0;
            _dGgammady[_qp] = 0.0;
            _dGgammadz[_qp] = 0.0;
                _dGgammadx_EN[_qp] = 0.0;
                _dGgammady_EN[_qp] = 0.0;
                _dGgammadz_EN[_qp] = 0.0;


            _dGgammadxplus[_qp] = 0.0;
            _dGgammadyplus[_qp] = 0.0;
            _dGgammadzplus[_qp] =  0.0;
                _dGgammadxplus_EN[_qp] = 0.0;
                _dGgammadyplus_EN[_qp] = 0.0;
                _dGgammadzplus_EN[_qp] = 0.0;


            _Ggammabar[_qp] = 0.0;
            _Ggammamin2grains[_qp] = 0.0;
            _TotGauss[_qp] = 0.0;
          }
      else  if (_Ggamma[_qp] == 0.0)
          {
            _L[_qp] = _L_BASE;
            _L_EN[_qp] = MetaPhysicL::raw_value(_L_BASE * 1);

            _Ggamma[_qp] = _GgammaBASE;
              _Ggamma_EN[_qp] = _GgammaBASE;

            _dGgammadx[_qp] = 0.0;
            _dGgammady[_qp] = 0.0;
            _dGgammadz[_qp] = 0.0;
                _dGgammadx_EN[_qp] = 0.0;
                _dGgammady_EN[_qp] = 0.0;
                _dGgammadz_EN[_qp] = 0.0;


            _dGgammadxplus[_qp] = 0.0;
            _dGgammadyplus[_qp] = 0.0;
            _dGgammadzplus[_qp] =  0.0;
                _dGgammadxplus_EN[_qp] = 0.0;
                _dGgammadyplus_EN[_qp] = 0.0;
                _dGgammadzplus_EN[_qp] = 0.0;


            _Ggammabar[_qp] = 0.0;
            _Ggammamin2grains[_qp] = 0.0;
            _TotGauss[_qp] = 0.0;
          }

        ADReal Ggamma = _Ggamma[_qp];


        //*************************************** Caluculation  of gamma and the derivatives *****************************************

        ADReal g2 = _Ggamma[_qp] * _Ggamma[_qp];

        ADReal p = -(3.0944*(g2*g2*g2*g2)) -(1.8169*(g2*g2*g2)) + (10.323*(g2*g2)) - (8.1819*(g2)) + 2.0033;

        ADReal dpdg2 = -((3.0944*4)*(g2*g2*g2)) -((1.8169*3)*(g2*g2)) + ((10.323*2)*(g2)) - 8.1819;

        _gamma[_qp] = 1/p;
          _gamma_EN[_qp] = MetaPhysicL::raw_value(1/p);

        ADReal gamma = _gamma[_qp];


        _dgammadx[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadx[_qp];
        _dgammady[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammady[_qp];
        _dgammadz[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadz[_qp];
          _dgammadx_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadx[_qp] );
          _dgammady_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammady[_qp] );
          _dgammadz_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadz[_qp] );


       _dgammadxplus[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadxplus[_qp];
       _dgammadyplus[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadyplus[_qp];
       _dgammadzplus[_qp] = (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadzplus[_qp];
         _dgammadxplus_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadxplus[_qp] );
         _dgammadyplus_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadyplus[_qp] );
         _dgammadzplus_EN[_qp] = MetaPhysicL::raw_value( (-2) * (_Ggamma[_qp]) * (_gamma[_qp]*_gamma[_qp]) * (dpdg2) * _dGgammadzplus[_qp] );


          if (sumval == 0.00)
            {
              _gamma[_qp] = _gammaBASE;
                _gamma_EN[_qp] = _gammaBASE;

              _dgammadx[_qp] = 0.0;
              _dgammady[_qp] = 0.0;
              _dgammadz[_qp] =  0.0;
                  _dgammadx_EN[_qp] = 0.0;
                  _dgammady_EN[_qp] = 0.0;
                  _dgammadz_EN[_qp] = 0.0;


             _dgammadxplus[_qp] = 0.0;
             _dgammadyplus[_qp] = 0.0;
             _dgammadzplus[_qp] =  0.0;
                 _dgammadxplus_EN[_qp] = 0.0;
                 _dgammadyplus_EN[_qp] = 0.0;
                 _dgammadzplus_EN[_qp] = 0.0;

            }
          else  if (_Ggamma[_qp] == 0.00)
            {
              _gamma[_qp] = _gammaBASE;
                _gamma_EN[_qp] = _gammaBASE;

              _dgammadx[_qp] = 0.0;
              _dgammady[_qp] = 0.0;
              _dgammadz[_qp] =  0.0;
                  _dgammadx_EN[_qp] = 0.0;
                  _dgammady_EN[_qp] = 0.0;
                  _dgammadz_EN[_qp] = 0.0;


             _dgammadxplus[_qp] = 0.0;
             _dgammadyplus[_qp] = 0.0;
             _dgammadzplus[_qp] =  0.0;
                 _dgammadxplus_EN[_qp] = 0.0;
                 _dgammadyplus_EN[_qp] = 0.0;
                 _dgammadzplus_EN[_qp] = 0.0;
            }

           //*************************************** Caluculation for lgb and sigma****************************************

        _sigma[_qp] = (_Ggamma[_qp]) *  (std::sqrt( _kappa[_qp] * _m[_qp] ));
            _sigma_EN[_qp] = MetaPhysicL::raw_value( (_Ggamma[_qp]) *  (std::sqrt( _kappa[_qp] * _m[_qp] )) );

        _sigmaORIUNIT[_qp] = (_sigma[_qp]) / (_JtoeV * _length_scale * _length_scale);
            _sigmaORIUNIT_EN[_qp] = MetaPhysicL::raw_value( (_sigma[_qp]) / (_JtoeV * _length_scale * _length_scale) );

        ADReal Inversegamma = 1/(_gamma[_qp]);
        ADReal Inversegamma1 = Inversegamma;
        ADReal Inversegamma2 = Inversegamma * Inversegamma;
        ADReal Inversegamma3 = Inversegamma * Inversegamma * Inversegamma;
        ADReal Inversegamma4 = Inversegamma * Inversegamma * Inversegamma * Inversegamma;
        ADReal Inversegamma5 = Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma;
        ADReal Inversegamma6 = Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma * Inversegamma;

        ADReal f0gamma = (0.0788 * Inversegamma6) - (0.4955 * Inversegamma5) + (1.2244 * Inversegamma4) - (1.5281 * Inversegamma3) + (1.0686 * Inversegamma2) - (0.5563 * Inversegamma1) + 0.2907;

        _lgb[_qp] = std::sqrt(_kappa[_qp] / (_m[_qp] * f0gamma));

            _lgb_EN[_qp] = MetaPhysicL::raw_value( (std::sqrt(_kappa[_qp] / (_m[_qp] * f0gamma))) );


  } // void bracket
