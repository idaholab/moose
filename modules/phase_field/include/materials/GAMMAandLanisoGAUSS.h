
// 5D Gaussian anisotropy material object
// Material properties adding anysotroopy to γ and L.


#pragma once

#include "ADMaterial.h"

class GrainTrackerInterface;

class GAMMAandLanisoGAUSS : public ADMaterial
{
public:
  static InputParameters validParams();

  GAMMAandLanisoGAUSS(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  ADMaterialProperty<Real> & _Ggamma;
    MaterialProperty<Real> & _Ggamma_EN;

  ADMaterialProperty<Real> & _dGgammadx;
    MaterialProperty<Real> & _dGgammadx_EN;
  ADMaterialProperty<Real> & _dGgammady;
    MaterialProperty<Real> & _dGgammady_EN;
  ADMaterialProperty<Real> & _dGgammadz;
    MaterialProperty<Real> & _dGgammadz_EN;

  ADMaterialProperty<Real> & _dGgammadxplus;
    MaterialProperty<Real> & _dGgammadxplus_EN;
  ADMaterialProperty<Real> & _dGgammadyplus;
    MaterialProperty<Real> & _dGgammadyplus_EN;
  ADMaterialProperty<Real> & _dGgammadzplus;
    MaterialProperty<Real> & _dGgammadzplus_EN;

  ADMaterialProperty<Real> & _VwlibR;
  ADMaterialProperty<Real> & _VxlibR;
  ADMaterialProperty<Real> & _VylibR;
  ADMaterialProperty<Real> & _VzlibR;

  ADMaterialProperty<Real> & _Vx;
  ADMaterialProperty<Real> & _Vy;
  ADMaterialProperty<Real> & _Vz;

  ADMaterialProperty<Real> & _gamma;
    MaterialProperty<Real> & _gamma_EN;

  ADMaterialProperty<Real> & _S_switch;

  ADMaterialProperty<Real> & _dgammadx;
    MaterialProperty<Real> & _dgammadx_EN;
  ADMaterialProperty<Real> & _dgammady;
    MaterialProperty<Real> & _dgammady_EN;
  ADMaterialProperty<Real> & _dgammadz;
    MaterialProperty<Real> & _dgammadz_EN;

  ADMaterialProperty<Real> & _dgammadxplus;
    MaterialProperty<Real> & _dgammadxplus_EN;
  ADMaterialProperty<Real> & _dgammadyplus;
    MaterialProperty<Real> & _dgammadyplus_EN;
  ADMaterialProperty<Real> & _dgammadzplus;
    MaterialProperty<Real> & _dgammadzplus_EN;

  ADMaterialProperty<Real> & _m;
    MaterialProperty<Real> & _m_EN;

  ADMaterialProperty<Real> & _kappa;
    MaterialProperty<Real> & _kappa_EN;

  ADMaterialProperty<Real> & _L;
    MaterialProperty<Real> & _L_EN;

  ADMaterialProperty<Real> & _lgb;
    MaterialProperty<Real> & _lgb_EN;

  const bool & _ADDGaussian;

  const bool & _ADDGaussianL;

  ADMaterialProperty<Real> & _sigma;
    MaterialProperty<Real> & _sigma_EN;
  ADMaterialProperty<Real> & _sigmaORIUNIT;
    MaterialProperty<Real> & _sigmaORIUNIT_EN;

  ADMaterialProperty<Real> & _qwg;

  ADMaterialProperty<Real> & _qxg;

  ADMaterialProperty<Real> & _qyg;

  ADMaterialProperty<Real> & _qzg;

  ADMaterialProperty<Real> & _Ggammabar;

  ADMaterialProperty<Real> & _Ggammamin2grains;

  ADMaterialProperty<Real> & _TotGauss;

  ADMaterialProperty<Real> & _MGBVALUE;

  const Real  _sigmaBASE;

  const Real  _GgammaBASE;

  const Real  _gammaBASE;

  const Real  _f0gammaBASE;

  const Real  _L_BASE;

  const Real  _lgbBASE_minimum;

  const Real  _alphaswitch;

  const Real  _betaswitch;

  unsigned int  _libnum;

  const Real  _amplitudeScale;

  const Real  _sharpness;

  const Real  _Gaussian_Tolerance;

  const FileName _Library_file_name;

  const FileName _Quaternion_file_name;

  const GrainTrackerInterface & _grain_tracker;

  const Real _length_scale;

  const Real _time_scale;

  const Real _JtoeV;

  const Real  _BoundaryNormal;

  const unsigned int _op_num;

  const std::vector<const ADVariableValue *> _vals;

  const std::vector<const ADVariableGradient *> _grad_vals;

  const MaterialPropertyName _Ggamma_name;
    const MaterialPropertyName _Ggamma_EN_name;

  const MaterialPropertyName _dGgammadx_name;
    const MaterialPropertyName _dGgammadx_EN_name;

  const MaterialPropertyName _dGgammady_name;
    const MaterialPropertyName _dGgammady_EN_name;

  const MaterialPropertyName _dGgammadz_name;
    const MaterialPropertyName _dGgammadz_EN_name;

  const MaterialPropertyName _dGgammadxplus_name;
    const MaterialPropertyName _dGgammadxplus_EN_name;
  const MaterialPropertyName _dGgammadyplus_name;
    const MaterialPropertyName _dGgammadyplus_EN_name;
  const MaterialPropertyName _dGgammadzplus_name;
    const MaterialPropertyName _dGgammadzplus_EN_name;

  const MaterialPropertyName _gamma_name;
    const MaterialPropertyName _gamma_EN_name;

  const MaterialPropertyName _S_switch_name;

  const MaterialPropertyName _dgammadx_name;
    const MaterialPropertyName _dgammadx_EN_name;

  const MaterialPropertyName _dgammady_name;
    const MaterialPropertyName _dgammady_EN_name;

  const MaterialPropertyName _dgammadz_name;
    const MaterialPropertyName _dgammadz_EN_name;

  const MaterialPropertyName _dgammadxplus_name;
    const MaterialPropertyName _dgammadxplus_EN_name;
  const MaterialPropertyName _dgammadyplus_name;
    const MaterialPropertyName _dgammadyplus_EN_name;
  const MaterialPropertyName _dgammadzplus_name;
    const MaterialPropertyName _dgammadzplus_EN_name;

  const MaterialPropertyName _m_name;
    const MaterialPropertyName _m_EN_name;

  const MaterialPropertyName _kappa_name;
    const MaterialPropertyName _kappa_EN_name;

  const MaterialPropertyName _L_name;
    const MaterialPropertyName _L_EN_name;

  const MaterialPropertyName _lgb_name;
    const MaterialPropertyName _lgb_EN_name;

        unsigned int _grain_num;

};


// %gamma		g(gamma)	f_0_max(gamma)
// %
// gamma_function =
// [0.520000	 0.098546	 0.004921
// 0.530000	 0.119839	 0.007320
// 0.540000	 0.137421	 0.009678
// 0.550000	 0.152600	 0.011994
// 0.600000	 0.208989	 0.022953
// 0.650000	 0.248543	 0.032941
// 0.700000	 0.279304	 0.042058
// 0.750000	 0.304484	 0.050483
// 0.800000	 0.325758	 0.058176
// 0.850000	 0.344131	 0.065273
// 0.900000	 0.360261	 0.071842
// 0.950000	 0.374603	 0.077943
// 1.000000	 0.387487	 0.083625
// 1.050000	 0.399151	 0.088995
// 1.100000	 0.409808	 0.093976
// 1.150000	 0.419591	 0.098654
// 1.200000	 0.428620	 0.103059
// 1.250000	 0.436994	 0.107215
// 1.300000	 0.444791	 0.111145
// 1.350000	 0.452079	 0.114869
// 1.400000	 0.458913	 0.118402
// 1.450000	 0.465341	 0.121762
// 1.500000	 0.471404	 0.124961
// 1.550000	 0.477137	 0.128012
// 1.600000	 0.482570	 0.130925
// 1.650000	 0.487730	 0.133712
// 1.700000	 0.492640	 0.136380
// 1.750000	 0.497320	 0.138937
// 1.800000	 0.501788	 0.141392
// 1.850000	 0.506062	 0.143750
// 1.900000	 0.510155	 0.146018
// 1.950000	 0.514080	 0.148201
// 2.000000	 0.517849	 0.150305
// 2.050000	 0.521473	 0.152355
// 2.100000	 0.524961	 0.154314
// 2.150000	 0.528321	 0.156207
// 2.200000	 0.531563	 0.158036
// 2.250000	 0.534692	 0.159806
// 2.300000	 0.537716	 0.161520
// 2.350000	 0.540641	 0.163180
// 2.400000	 0.543472	 0.164790
// 2.450000	 0.546215	 0.166351
// 2.500000	 0.548873	 0.167866
// 2.550000	 0.551452	 0.169338
// 2.600000	 0.553956	 0.170768
// 2.650000	 0.556388	 0.172159
// 2.700000	 0.558752	 0.173511
// 2.750000	 0.561051	 0.174827
// 2.800000	 0.563289	 0.176109
// 2.850000	 0.565468	 0.177357
// 2.900000	 0.567590	 0.178574
// 2.950000	 0.569659	 0.179760
// 3.000000	 0.571676	 0.180917
// 3.050000	 0.573644	 0.182046
// 3.100000	 0.575565	 0.183148
// 3.150000	 0.577441	 0.184225
// 3.200000	 0.579274	 0.185276
// 3.250000	 0.581064	 0.186304
// 3.300000	 0.582815	 0.187308
// 3.350000	 0.584528	 0.188290
// 3.400000	 0.586203	 0.189251
// 3.450000	 0.587843	 0.190191
// 3.500000	 0.589449	 0.191112
// 3.550000	 0.591022	 0.192013
// 3.600000	 0.592563	 0.192895
// 3.650000	 0.594073	 0.193760
// 3.700000	 0.595553	 0.194607
// 3.750000	 0.597005	 0.195438
// 3.800000	 0.598429	 0.196252
// 3.850000	 0.599826	 0.197051
// 3.900000	 0.601197	 0.197834
// 3.950000	 0.602543	 0.198603
// 4.000000	 0.603865	 0.199357
// 4.050000	 0.605118	 0.200150
// 4.100000	 0.606394	 0.200878
// 4.150000	 0.607647	 0.201592
// 4.200000	 0.608879	 0.202294
// 4.250000	 0.610090	 0.202984
// 4.300000	 0.611281	 0.203662
// 4.350000	 0.612452	 0.204328
// 4.400000	 0.613604	 0.204983
// 4.450000	 0.614738	 0.205627
// 4.500000	 0.615854	 0.206260
// 4.550000	 0.616952	 0.206883
// 4.600000	 0.618033	 0.207496
// 4.650000	 0.619098	 0.208100
// 4.700000	 0.620146	 0.208693
// 4.750000	 0.621179	 0.209277
// 4.800000	 0.622197	 0.209853
// 4.850000	 0.623200	 0.210419
// 4.900000	 0.624188	 0.210977
// 4.950000	 0.625162	 0.211526
// 5.000000	 0.626122	 0.212068
// 5.050000	 0.627069	 0.212601
// 5.100000	 0.628003	 0.213126
// 5.150000	 0.628924	 0.213644
// 5.200000	 0.629832	 0.214155
// 5.250000	 0.630728	 0.214658
// 5.300000	 0.631613	 0.215154
// 5.350000	 0.632485	 0.215643
// 5.400000	 0.633347	 0.216126
// 5.450000	 0.634197	 0.216602
// 5.500000	 0.635036	 0.217071
// 5.550000	 0.635865	 0.217534
// 5.600000	 0.636683	 0.217991
// 5.650000	 0.637492	 0.218442
// 5.700000	 0.638290	 0.218887
// 5.750000	 0.639078	 0.219326
// 5.800000	 0.639858	 0.219760
// 5.850000	 0.640627	 0.220188
// 5.900000	 0.641388	 0.220611
// 5.950000	 0.642140	 0.221028
// 6.000000	 0.642882	 0.221440
// 6.050000	 0.643617	 0.221847
// 6.100000	 0.644343	 0.222250
// 6.150000	 0.645061	 0.222647
// 6.200000	 0.645770	 0.223039
// 6.250000	 0.646472	 0.223427
// 6.300000	 0.647166	 0.223810
// 6.350000	 0.647853	 0.224189
// 6.400000	 0.648531	 0.224563
// 6.450000	 0.649203	 0.224933
// 6.500000	 0.649868	 0.225299
// 6.550000	 0.650525	 0.225660
// 6.600000	 0.651176	 0.226018
// 6.650000	 0.651819	 0.226371
// 6.700000	 0.652457	 0.226721
// 6.750000	 0.653087	 0.227066
// 6.800000	 0.653711	 0.227408
// 6.850000	 0.654329	 0.227746
// 6.900000	 0.654941	 0.228081
// 6.950000	 0.655546	 0.228411
// 7.000000	 0.656146	 0.228739
// 7.050000	 0.656740	 0.229063
// 7.100000	 0.657328	 0.229383
// 7.150000	 0.657910	 0.229700
// 7.200000	 0.658486	 0.230014
// 7.250000	 0.659058	 0.230324
// 7.300000	 0.659623	 0.230632
// 7.350000	 0.660184	 0.230936
// 7.400000	 0.660739	 0.231237
// 7.450000	 0.661289	 0.231535
// 7.500000	 0.661835	 0.231831
// 7.550000	 0.662375	 0.232123
// 7.600000	 0.662910	 0.232412
// 7.650000	 0.663440	 0.232699
// 7.700000	 0.663966	 0.232983
// 7.750000	 0.664487	 0.233264
// 7.800000	 0.665004	 0.233542
// 7.850000	 0.665516	 0.233818
// 7.900000	 0.666023	 0.234091
// 7.950000	 0.666526	 0.234361
// 8.000000	 0.667025	 0.234629
// 8.100000	 0.668068	 0.235154
// 8.200000	 0.669037	 0.235673
// 8.300000	 0.669990	 0.236182
// 8.400000	 0.670927	 0.236683
// 8.500000	 0.671849	 0.237174
// 8.600000	 0.672757	 0.237657
// 8.700000	 0.673650	 0.238132
// 8.800000	 0.674529	 0.238599
// 8.900000	 0.675395	 0.239057
// 9.000000	 0.676247	 0.239509
// 9.100000	 0.677087	 0.239952
// 9.200000	 0.677915	 0.240389
// 9.300000	 0.678730	 0.240818
// 9.400000	 0.679533	 0.241241
// 9.500000	 0.680325	 0.241657
// 9.600000	 0.681106	 0.242066
// 9.700000	 0.681876	 0.242469
// 9.800000	 0.682635	 0.242866
// 9.900000	 0.683384	 0.243257
// 10.000000	 0.684122	 0.243642
// 11.000000	 0.690998	 0.247201
// 12.000000	 0.697079	 0.250305
// 13.000000	 0.702515	 0.253045
// 14.000000	 0.707419	 0.255487
// 15.000000	 0.711877	 0.257680
// 16.000000	 0.715955	 0.259665
// 20.000000	 0.729414	 0.266055
// 24.000000	 0.739720	 0.270773
// 32.000000	 0.754840	 0.277395
// 40.000000	 0.765691	 0.281913];
//
