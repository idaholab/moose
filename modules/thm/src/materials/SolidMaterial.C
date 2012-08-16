#include "SolidMaterial.h"

template<>
InputParameters validParams<SolidMaterial>()
{
  InputParameters params = validParams<Material>();

  // Coupled variables
  params.addRequiredCoupledVar("tw", "");
  params.addRequiredParam<std::string>("name_of_hs", "heat structure name");
  return params;
}

SolidMaterial::SolidMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _specific_heat(declareProperty<Real>("specific_heat")),
    _density(declareProperty<Real>("density")),
    _tw(coupledValue("tw")),
    _name_of_hs(getParam<std::string>("name_of_hs"))
{
}

void SolidMaterial::computeProperties()
{
   Real K=0.0;
   Real Cp=0.0;
   Real Rho = 0.0;
   for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
   {
    if (_name_of_hs == "fuel")
       fuelProperties(K, Rho, Cp, _tw[qp]);
    else if (_name_of_hs =="gap")
       gapProperties(K, Rho, Cp, _tw[qp]);
    else if (_name_of_hs == "clad")
       cladProperties(K, Rho, Cp, _tw[qp]);
    else
       mooseError("Heat structure is not specified");

     _thermal_conductivity[qp] = K;
     _density[qp] = Rho;
     _specific_heat[qp] = Cp;
     //std::cout<<"name_of_hs="<< _name_of_hs<<std::endl;
     //std::cout<<"qp="<<qp<<" tw="<<_tw[qp]<<" conductivity="<<_thermal_conductivity[qp]<<" density="<<_density[qp]<<"  Cp="<<_specific_heat[qp]<<std::endl;
   } 
}

void SolidMaterial::gapProperties(Real & K, Real & Rho, Real & Cp, Real Temp)
{

  Real T_gap[] = {5.0,  300.0,  400.0,  500.0,  600.0,  700.0,  800.0,  900.0,  1000.0,
               1100.0, 1200.0, 1300.0, 1400.0, 1500.0, 1600.0, 1700.0, 1800.0,  1900.0,
               2000.0, 2100.0, 2200.0, 2300.0, 2400.0, 2500.0, 2600.0, 2700.0,  2800.0,
               2900.0, 3000.0, 5000.0};
  Real K_gap[] = {0.0104, 0.0104, 0.0132, 0.0159, 0.0185, 0.0211, 0.0236, 0.0260, 0.0284,
                  0.0307, 0.0330, 0.0353, 0.0376, 0.0398, 0.0420, 0.0442, 0.0463, 0.0485,
                  0.0506, 0.0527, 0.0548, 0.0568, 0.0589, 0.0609, 0.0630, 0.0650, 0.0670,
                  0.0690, 0.0709, 0.0709};
  
  unsigned int N = sizeof(K_gap)/sizeof(K_gap[0]);
  if (Temp <= T_gap[0])
     K = K_gap[0];
  else if (Temp >=T_gap[N-1])
     K = K_gap[N-1];
  else
  {
     unsigned int i=1;
     for (; i<N; i++)
     {
       if (Temp<=T_gap[i])
         break;
     }
       K = ((Temp-T_gap[i-1])*K_gap[i] + (T_gap[i]-Temp)*K_gap[i-1])/(T_gap[i]-T_gap[i-1]);
  }
  Rho = 183.06;
  Cp =  186.65;
  return;
}

void SolidMaterial::fuelProperties(Real & K, Real & Rho, Real & Cp, Real Temp)
{
  std::vector<Real> T_Fuel_K(35), K_Fuel(35), T_Fuel_RhoCp(33), RhoCp_Fuel(33);

  Real T_fuel_K[] = { 5.0,  300.0,  400.0,  500.0,  600.0,  700.0,  800.0,  900.0, 1000.0,
                 1100.0, 1200.0, 1300.0, 1364.0, 1400.0, 1500.0, 1600.0, 1700.0, 1800.0,
                 1877.0, 1900.0, 2000.0, 2100.0, 2200.0, 2300.0, 2400.0, 2500.0, 2600.0,
                 2700.0, 2800.0, 2900.0, 3000.0, 3100.0, 3113.0, 3114.0, 5000.0};
  Real K_fuel[] = {8.284, 8.284, 7.086, 6.087, 5.316, 4.721, 4.255, 3.882, 3.580, 3.330,
                   3.123, 2.951, 2.855, 2.794, 2.642, 2.515, 2.411, 2.330, 2.327, 2.322,
                   2.308, 2.310, 2.328, 2.363, 2.462, 2.576, 2.706, 2.852, 3.014, 3.193,
                   3.388, 3.600, 3.629, 11.50, 11.50};

  unsigned int N = sizeof(K_fuel)/sizeof(K_fuel[0]);
  if (Temp <= T_fuel_K[0])
     K = K_fuel[0];
  else if (Temp >=T_fuel_K[N-1])
     K = K_fuel[N-1];
  else
  {
     unsigned int i=1;
     for (; i<N; i++)
     {
       if (Temp<=T_fuel_K[i])
         break;
     }
     K = ((Temp-T_fuel_K[i-1])*K_fuel[i] + (T_fuel_K[i]-Temp)*K_fuel[i-1])/(T_fuel_K[i]-T_fuel_K[i-1]);
  }

  Real T_fuel_Cp[] = { 5.0,   300.0,  400.0,  500.0,  600.0,  700.0,  800.0,  900.0, 1000.0,
                      1100.0, 1200.0, 1300.0, 1400.0, 1500.0, 1600.0, 1700.0, 1800.0, 1900.0,
                      2000.0, 2100.0, 2200.0, 2300.0, 2400.0, 2500.0, 2600.0, 2700.0, 2800.0,
                      2900.0, 3000.0, 3100.0, 3113.0, 3114.0, 5000.0};
  Real Cp_fuel[] = {236.339, 236.339, 265.847, 282.058, 292.350, 299.636, 305.282, 310.018,
                    314.026, 317.668, 321.129, 324.590, 328.233, 332.423, 337.432, 343.807,
                    351.821, 362.113, 375.046, 391.075, 410.474, 433.424, 460.200, 490.893,
                    525.410, 563.752, 605.647, 651.093, 699.727, 751.366, 758.288, 503.005,
                    503.005};

  unsigned int M = sizeof(Cp_fuel)/sizeof(Cp_fuel[0]);
  if (Temp <= T_fuel_Cp[0])
     Cp = Cp_fuel[0];
  else if (Temp >=T_fuel_Cp[M-1])
     Cp = Cp_fuel[M-1];
  else
  {
     unsigned int i=1;
     for (; i<M; i++)
     {
       if (Temp<=T_fuel_Cp[i])
         break;
     }
     Cp = ((Temp-T_fuel_Cp[i-1])*Cp_fuel[i] + (T_fuel_Cp[i]-Temp)*Cp_fuel[i-1])/(T_fuel_Cp[i]-T_fuel_Cp[i-1]);
  }

  Rho = 10980.0;
  return;
}


void SolidMaterial::cladProperties(Real & K, Real & Rho, Real & Cp, Real Temp)
{
  Real T_clad_K[] = { 5.0,  300.0,  400.0,  500.0,  600.0,  700.0,  800.0,  900.0, 1000.0,
                 1100.0, 1200.0, 1300.0, 1400.0, 1500.0, 1600.0, 1700.0, 1800.0,
                 1900.0, 2000.0, 2098.0, 2125.0, 5000.0};

  Real K_clad[] = {12.68, 12.68,  14.04,  15.29,  16.49,  17.67,  18.88,  20.17,  21.58,
                   23.16, 24.96,  27.03,  29.40,  32.12,  35.25,  38.82,  42.88,  47.48,
                   52.67, 58.36,  36.00,  36.00};

  unsigned int N = sizeof(K_clad)/sizeof(K_clad[0]);
  if (Temp <= T_clad_K[0])
     K = K_clad[0];
  else if (Temp >=T_clad_K[N-1])
     K = K_clad[N-1];
  else
  {
     unsigned int i=1;
     for (; i<N; i++)
     {
       if (Temp<=T_clad_K[i])
         break;
     }
       K = ((Temp-T_clad_K[i-1])*K_clad[i] + (T_clad_K[i]-Temp)*K_clad[i-1])/(T_clad_K[i]-T_clad_K[i-1]);
  }

  Real T_clad_Cp[] = { 5.0,   300.0,  400.0, 640.0, 1090.0,  1093.0,  1113.0,  1133.0,
                      1153.0,  1173.0,  1193.0,  1213.0,  1233.0,  1248.0,  5000.0};
  Real Cp_clad[] = {281.026, 281.026, 301.939, 330.942, 375.057, 502.061, 589.986, 615.021,
                    718.974,  816.059, 769.959, 618.989, 468.936, 355.976, 355.976};

  unsigned int M = sizeof(Cp_clad)/sizeof(Cp_clad[0]);
  if (Temp <= T_clad_Cp[0])
     Cp = Cp_clad[0];
  else if (Temp >=T_clad_Cp[M-1])
     Cp = Cp_clad[M-1];
  else
  {
     unsigned int i=1;
     for (; i<M; i++)
     {
       if (Temp<=T_clad_Cp[i])
         break;
     }
     Cp = ((Temp-T_clad_Cp[i-1])*Cp_clad[i] + (T_clad_Cp[i]-Temp)*Cp_clad[i-1])/(T_clad_Cp[i]-T_clad_Cp[i-1]);
  }

  Rho = 6551.0;
  return;
}

void SolidMaterial::residualSetup()
{
}
