/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "WaterSteamEOS.h"

///  UNITS:
///  pressure - [Pa]
///  enthalpy - [J/kg]
///  tempurature - [K]
///  density - [kg/m3]
///  viscosity - [Pa.s]

template<>
InputParameters validParams<WaterSteamEOS>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}

WaterSteamEOS::WaterSteamEOS(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params)
{ }

WaterSteamEOS::~WaterSteamEOS()
{ }

//Suplimentary functions used within the two main functions bellow (Equations_of_State_Properties and Equations_of_State_Derivative_Properties):
Real WaterSteamEOS::phaseDetermine (Real enth_in, Real press_in, Real& phase, Real& temp_sat, Real& enth_water_sat, Real& enth_steam_sat, Real& dens_water_sat, Real& dens_steam_sat) const
{
  ///////////This funtion determines what region of the pressure/temperature graph the water is in (compressed water, steam,
  ///////////or saturated mixture) using the gibbs free energy equations and the 'IAPWS Industrial Formulation 1997 for the
  ///////////Thermodynamic Properties of Water and Steam'.
  ///////////This function is only valid for regions 1, 2, and 4 (compressed water, steam, and the saturation curve) and is
  ///////////only acurate for temperatures bellow 623.15 K (350 C), pressures bellow 100 MPa, and enthalpies bellow 2.6 MJ/kg

  //VARIABLES:
  Real rconst= 0.461526e3;                                           //Gas constant
  Real press_sat_350C = 16.529e6;                                    //saturated pressure at 350C
  Real enth_water_350C = 1670.9e3;                                   //enthalpy of water at 350C
  //Part I variables (dertermining saturation temp of region 4)-
  Real beta, beta2;
  Real m, n, q, r, x;                                                //dummy terms used in temp_sat algebra *formerly d, e, f, g
  Real pstar = 1.0e6;                                                //shifting factor for pressure in region 4, p*
  Real nr[10] = { 0.11670521452767e4, -0.72421316703206e6, -0.17073846940092e2,
                  0.12020824702470e5, -0.32325550322333e7, 0.14915108613530e2,
                  -0.48232657361591e4, 0.40511340542057e6, -0.23855557567849e0,
                  0.65017534844798e3 };

  //Part II variables (determining saturated liquid enthalpy and density of region 1)-
  Real ttol = 1.0e-3;                                                //this tolerance allows the function to be called
  //just outside its nominal operating range
  //when doing transitions to supercritical

  Real gamma_pie1 = 0.0;                                             //partial derivative of region 1 gibbs equation w.r.t. pressure term
  Real gamma_tau1 = 0.0;                                             //partial derivative of region 1 gibbs equation w.r.t. temperature term
  Real pie1 = 0.0;                                                   //shifted value of pressure in region 1, =p/p*
  Real tau1 = 0.0;                                                   //shifted value of temperature in region 1, =T*/T
  Real rt1 = 0.0;                                                    //gas constant * temperature, RT from ideal gas law
  Real pstar1 = 16.53e6;                                             //pressure shifting term in regon 1, p*
  Real tstar1 = 1386.e0;                                             //temperature shifting term in region 1, T*
  Real intern_energy_water;                                          //*formerly u
  Real n1[34] = { 0.14632971213167e0, -0.84548187169114e0, -0.37563603672040e1,
                  0.33855169168385e1, -0.95791963387872e0, 0.15772038513228e0,
                  -0.16616417199501e-1,0.81214629983568e-3, 0.28319080123804e-3,
                  -0.60706301565874e-3, -0.18990068218419e-1,-0.32529748770505e-1,
                  -0.21841717175414e-1, -0.52838357969930e-4, -0.47184321073267e-3,
                  -0.30001780793026e-3, 0.47661393906987e-4, -0.44141845330846e-5,
                  -0.72694996297594e-15,-0.31679644845054e-4, -0.28270797985312e-5,
                  -0.85205128120103e-9, -0.22425281908000e-5,-0.65171222895601e-6,
                  -0.14341729937924e-12, -0.40516996860117e-6, -0.12734301741641e-8,
                  -0.17424871230634e-9, -0.68762131295531e-18, 0.14478307828521e-19,
                  0.26335781662795e-22,-0.11947622640071e-22, 0.18228094581404e-23,
                  -0.93537087292458e-25 };                                        //n coefficients for region 1 gibbs equation
  Real I1[34] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2,
                  2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29,
                  30, 31, 32 };                                                   //I coefficients for region 1 gibbs equation
  Real J1[34] = { -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3,
                  0, 1, 3, 17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29,
                  -31, -38, -39, -40, -41 };                                      //J coefficients for region 1 gibbs equation

  //Part III variables (determining saturated steam enthalpy and density of region 2)-
  Real gamma_pie2 = 0.0;                                             //total partial derivative of region 2 gibbs equation w.r.t. pressure term
  // = gamma_pie2_0 + gamma_pie2_r, (ideal gas + residual)
  Real gamma_tau2 = 0.0;                                             //total partial derivative of region 2 gibbs equation w.r.t. temperature term
  // = gamma_tau2_0 + gamma_tau2_r, (ideal gas + residual)
  Real gamma_pie2_0 = 0.0;                                           //ideal gas partial derivative term of region 2 gibbs eq. w.r.t. pressure term
  Real gamma_tau2_0 = 0.0;                                           //ideal gas partial derivative term of region 2 gibbs eq. w.r.t. temperature term
  Real gamma_pie2_r = 0.0;                                           //residual partial derivative term of region 2 gibbs eq. w.r.t. pressure term
  Real gamma_tau2_r = 0.0;                                           //residual partial derivative term of region 2 gibbs eq. w.r.t. temperature term
  Real pie2 = 0.0;                                                   //shifted value of pressure in region 2, =p/p*
  Real tau2 = 0.0;                                                   //shifted value of temperature in region 2, =T*/T
  Real rt2 = 0.0;                                                    //gas constant * temperature, RT from ideal gas law
  Real pstar2 = 1.0e6;                                               //pressure shifting term in region 2, p*
  Real tstar2 = 540.e0;                                              //temperature shifting term in region 2, T*
  Real intern_energy_steam;                                          //*formerly u
  Real n2_0[9] = { -0.96927686500217e1, 0.10086655968018e2,-0.56087911283020e-2,
                   0.71452738081455e-1, -0.40710498223928e0, 0.14240819171444e1,
                   -0.43839511319450e1,  -0.28408632460772e0, 0.21268463753307e-1 };   //n coefficients for region 2 gibbs eq. ideal gas term
  Real J2_0[9] = { 0, 1, -5, -4, -3, -2, -1, 2, 3 };                      //J coefficients for region 2 gibbs eq. ideal gas term
  Real n2_r[43] = { -0.17731742473213e-2, -0.17834862292358e-1,-0.45996013696365e-1,
                    -0.57581259083432e-1, -0.50325278727930e-1, -0.33032641670203e-4,
                    -0.18948987516315e-3, -0.39392777243355e-2, -0.43797295650573e-1,
                    -0.26674547914087e-4, 0.20481737692309e-7, 0.43870667284435e-6,
                    -0.32277677238570e-4, -0.15033924542148e-2, -0.40668253562649e-1,
                    -0.78847309559367e-9, 0.12790717852285e-7, 0.48225372718507e-6,
                    0.22922076337661e-5, -0.16714766451061e-10, -0.21171472321355e-2,
                    -0.23895741934104e2, -0.59059564324270e-17,-0.12621808899101e-5,
                    -0.38946842435739e-1,  0.11256211360459e-10, -0.82311340897998e1,
                    0.19809712802088e-7, 0.10406965210174e-18,-0.10234747095929e-12,
                    -0.10018179379511e-8, -0.80882908646985e-10, 0.10693031879409e0,
                    -0.33662250574171e0, 0.89185845355421e-24, 0.30629316876232e-12,
                    -0.42002467698208e-5, -0.59056029685639e-25, 0.37826947613457e-5,
                    -0.12768608934681e-14, 0.73087610595061e-28, 0.55414715350778e-16,
                    -0.94369707241210e-6 };                                             //n coefficients for region 2 gibbs eq. residual term
  Real J2_r[43] = { 0, 1, 2, 3, 6, 1, 2, 4, 7, 36, 0, 1,
                    3, 6, 35, 1, 2, 3, 7, 3, 16, 35, 0,
                    11, 25, 8, 36, 13, 4, 10, 14, 29, 50,
                    57, 20, 35, 48, 21, 53, 39, 26, 40, 58 };                           //J coefficients for region 2 gibbs eq. residual term
  Real I2_r[43] = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3,
                    3, 3, 3, 4, 4, 4, 5, 6, 6, 6, 7, 7,
                    7, 8, 8, 9, 10, 10, 10, 16, 16, 18,
                    20, 20, 20, 21, 22, 23, 24, 24, 24 };                               //I coefficients for region 2 gibbs eq. residual term


  ////////PART I:
  //Determine the saturation temp given pressure and enthalpy
  //using region 4 saturatuion-pressure equation (formerly tsat function):
  beta2 = sqrt(press_in / pstar);
  beta = sqrt(beta2);
  n = beta2 + nr[2] * beta + nr[5];
  q = nr[0] * beta2 + nr[3] * beta + nr[6];
  r = nr[1] * beta2 + nr[4] * beta + nr[7];
  m = 2.0e0 * r / (-q - sqrt(q * q - 4.e0 * n * r));
  x = nr[9] + m;
  temp_sat = 0.5e0 * (nr[9] + m - sqrt(x * x - 4.e0 * (nr[8] + nr[9] * m)));

  ////////PART II:
  //Determine the density and enthapy of cold water
  //using region 1 gibbs free energy equation (formerly cowat function):
  //input - temp_sat, press_in
  //output - enth_water
  if ((temp_sat <= 623.15 + ttol) && (press_in <= 100.e6))
  {
    pie1 = press_in / pstar1;
    tau1 = tstar1 / temp_sat;

    //gibbs free energy terms for region 1
    for (int i=0; i<34; i++)
    {
      gamma_pie1 = gamma_pie1 - n1[i] * I1[i] * pow((7.1 - pie1),(I1[i] - 1)) * pow((tau1 - 1.222),(J1[i]));
      gamma_tau1 = gamma_tau1 + n1[i] * J1[i] * pow((7.1 - pie1),(I1[i])) * pow((tau1 - 1.222),(J1[i] - 1));
    }

    rt1 = rconst * temp_sat;
    dens_water_sat = pstar1 / (rt1 * gamma_pie1);
    intern_energy_water = rt1 * (tau1 * gamma_tau1 - pie1 * gamma_pie1);
    enth_water_sat = intern_energy_water + press_in / dens_water_sat;

    ///////PART III:
    //Determine the density and enthapy of superheated steam
    //using region 2 gibbs free energy equation (formerly supst function):
    //input - temp_sat, press_in
    //output - enth_steam
    if ((temp_sat <= 1273.15) && (press_in <= 100.0e6))
    {
      pie2 = press_in / pstar2;
      tau2 = tstar2 / temp_sat;

      //gibbs ideal gas terms for region 2
      gamma_pie2_0 = pow(pie2 , -1);

      for (int i=0; i<9; i++)
      {
        gamma_tau2_0 = gamma_tau2_0 + n2_0[i] * J2_0[i] * pow(tau2, (J2_0[i] - 1));
      }

      //gibbs residual terms for region 2
      for (int i=0; i<43; i++)
      {
        gamma_pie2_r = gamma_pie2_r + n2_r[i] * I2_r[i] * pow(pie2 , (I2_r[i] - 1)) * pow((tau2 - 0.5) , J2_r[i]);
        gamma_tau2_r = gamma_tau2_r + n2_r[i] * J2_r[i] * pow(pie2 , I2_r[i]) * pow((tau2 - 0.5) , (J2_r[i] - 1));
      }

      //gibbs free energy terms (ideal gass + residual)
      gamma_pie2 = gamma_pie2_0 + gamma_pie2_r;
      gamma_tau2 = gamma_tau2_0 + gamma_tau2_r;

      rt2 = rconst * temp_sat;
      dens_steam_sat = pstar2 / (rt2 * gamma_pie2);
      intern_energy_steam = rt2 * (tau2 * gamma_tau2 - pie2 * gamma_pie2);
      enth_steam_sat = intern_energy_steam + press_in / dens_steam_sat;
    }
    else
    {
      mooseError("Either Saturation Temp. > 1273.15 K, or Pressure > 100 MPa, at the moment FALCON cannot handle such EXTREMES!!!");
    }
  }
  else
  {
    enth_water_sat = enth_water_350C;
    dens_water_sat = 760.0;
    intern_energy_water = 1442.0e3;
  }

  ////////PART IV:
  //Deterining the phase ([1] = Cold Water. [2] = Superheated Steam, [3] = Saturated Mixture
  if ((press_in > press_sat_350C) && (enth_in <= enth_water_350C))
  {
    phase = 1; //Cold water
  }
  else if (enth_in >= enth_steam_sat)
  {
    phase = 2; //Superheated steam
  }
  else if ((enth_in <= enth_steam_sat) && (enth_in >= enth_water_sat))
  {
    phase = 3; //Saturated mixture
  }
  else
  {
    phase = 1;
  }

  return (0);

}

Real WaterSteamEOS::waterEquationOfStatePH (Real enth_in, Real press_in, Real temp_in, Real temp_sat, Real& temp1, Real& dens1, Real& enth1) const
{
  //Variables:
  Real rconst= 0.461526e3;                                    //Gas constant
  int itr = 0.0;                                              //Newton iteration counter, will stop iterations if more that 150
  Real itr_temp1, itr_temp2;                                  //intermediate itteration temperature variables
  Real dt = -1.0e-7;                                          //temperature incrementation for each itteration
  Real enth1_x;                                               //intermediate itteration enthalpy variable, *formerly hx
  Real gamma_pie1 = 0.0;                                      //partial derivative w.r.t. pressure term for gibbs eq.
  Real gamma_tau1 = 0.0;                                      //partial derivative w.r.t. temperature term of gibbs eq.
  Real pie1;                                                  //shifted value of pressure, =p/p*
  Real tau1;                                                  //shifted value of temperature, =T*/T
  Real rt1;                                                   //gass constant * temperature, RT from ideal gas law
  Real pstar1 = 16.53e6;                                      //pressure shifting term, p*
  Real tstar1 = 1386.e0;                                      //temperature shifting term, T*
  Real intern_energy1;                                        //*formerly u
  Real n1[34] = { 0.14632971213167e0, -0.84548187169114e0, -0.37563603672040e1,
                  0.33855169168385e1, -0.95791963387872e0, 0.15772038513228e0,
                  -0.16616417199501e-1,0.81214629983568e-3, 0.28319080123804e-3,
                  -0.60706301565874e-3, -0.18990068218419e-1,-0.32529748770505e-1,
                  -0.21841717175414e-1, -0.52838357969930e-4, -0.47184321073267e-3,
                  -0.30001780793026e-3, 0.47661393906987e-4, -0.44141845330846e-5,
                  -0.72694996297594e-15,-0.31679644845054e-4, -0.28270797985312e-5,
                  -0.85205128120103e-9, -0.22425281908000e-5,-0.65171222895601e-6,
                  -0.14341729937924e-12, -0.40516996860117e-6, -0.12734301741641e-8,
                  -0.17424871230634e-9, -0.68762131295531e-18, 0.14478307828521e-19,
                  0.26335781662795e-22,-0.11947622640071e-22, 0.18228094581404e-23,
                  -0.93537087292458e-25 };                                //n coefficients of region 1 gibbs eq.
  Real I1[34] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2,
                  2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29,
                  30, 31, 32 };                                           //I coefficients of region 1 gibbs eq.
  Real J1[34] = { -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3,
                  0, 1, 3, 17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29,
                  -31, -38, -39, -40, -41 };                              //J coefficients of region 1 gibbs eq.
  Real dif;

  //Obtain an better initial guess for the output temp.
  //RKP edit August 2013, change the intial guess for temperature to be passed in from the application
  //itr_temp1 = temp_sat - 1.e-6;  //RKP commented out

  if (temp_in < 273.13)
    itr_temp1 = temp_sat - 1.e-6;  //in case temp_in is too low (i.e., below freezing), start at a resaonable point
  else
    itr_temp1 = temp_in;

  //Newton iterations of cold water density and enthalpy calculations for region 1
  //(formerly cowat iterations)
  //input - init_temp_guess and press_in
  //output - temp1, enth1, dens1


while (itr < 151)                                           //exits if Newton iteration does not converge within 150 times
  {
    ////Part 1 (fist itteration):

    //Initialize variables

    pie1 = press_in / pstar1;
    tau1 = tstar1 / itr_temp1;
    enth1 = 0.0;
    intern_energy1 = 0.0;
    dens1 = 0.0;
    rt1 = 0.0;
    gamma_pie1 = 0.0;
    gamma_tau1 = 0.0;

    //gibbs free energy terms for region 1
    for (int i=0; i<34; i++)
    {
      gamma_pie1 = gamma_pie1 - n1[i] * I1[i] * pow((7.1 - pie1),(I1[i] - 1)) * pow((tau1 - 1.222),(J1[i]));
      gamma_tau1 = gamma_tau1 + n1[i] * J1[i] * pow((7.1 - pie1),(I1[i])) * pow((tau1 - 1.222),(J1[i] - 1));
    }

    rt1 = rconst * itr_temp1;
    dens1 = pstar1 / (rt1 * gamma_pie1);
    intern_energy1 = rt1 * (tau1 * gamma_tau1 - pie1 * gamma_pie1);
    enth1 = intern_energy1 + press_in / dens1;

    dif = std:: abs(enth1 - enth_in);

    if (dif <= 1.0e-8)

    {
      break;
    }

    itr_temp2 = itr_temp1 + dt;

    ////Part 2 (second itteration):

    //Re-initialize variables
    pie1 = press_in / pstar1;
    tau1 = tstar1 / itr_temp2;
    intern_energy1 = 0.0;
    dens1 = 0.0;
    rt1 = 0.0;
    gamma_pie1 = 0.0;
    gamma_tau1 = 0.0;

    //gibbs free energy terms for region 1
    for (int i=0; i<34; i++)
    {
      gamma_pie1 = gamma_pie1 - n1[i] * I1[i] * pow((7.1 - pie1),(I1[i] - 1)) * pow((tau1 - 1.222),(J1[i]));
      gamma_tau1 = gamma_tau1 + n1[i] * J1[i] * pow((7.1 - pie1),(I1[i])) * pow((tau1 - 1.222),(J1[i] - 1));
    }

    rt1 = rconst * itr_temp2;
    dens1 = pstar1 / (rt1 * gamma_pie1);
    intern_energy1 = rt1 * (tau1 * gamma_tau1 - pie1 * gamma_pie1);
    enth1_x = intern_energy1 + press_in / dens1;
    itr_temp1 = itr_temp1 + (enth_in - enth1) * dt / (enth1_x - enth1);

    itr = itr + 1;

    if (itr > 150)
    {
      //Moose::out << "water_eq_of_state: Newton iteration does not converge within 150 times. STOP." << std::endl;
      break;
    }
  }

  temp1 = itr_temp1;

  return (0);
}

Real WaterSteamEOS::steamEquationOfStatePH (Real enth_in, Real press_in, Real temp_in, Real temp_sat, Real& temp2, Real& dens2, Real& enth2) const
{
  //Variables:
  int itr = 0.0;                                              //Newton iteration counter, will stop iterations if more that 150
  Real dt = 1.0e-7;                                           //temperature incrementation for each itteration
  Real itr_temp1, itr_temp2;                                  //intermediate itteration temperature variables
  Real rconst = 0.461526e3;                                   //Gass constant
  Real gamma_pie2 = 0.0;                                      //total partial derivative w.r.t. pressure term
  // = gamma_pie2_0 + gamma_pie2_r, (ideal gas + residual)
  Real gamma_tau2 = 0.0;                                      //total partial derivative w.r.t. temperature term
  // = gamma_tau2_0 + gamma_tau2_r, (ideal gas + residual)
  Real gamma_pie2_0 = 0.0;                                    //ideal gas partial derivative term w.r.t. pressure term
  Real gamma_tau2_0 = 0.0;                                    //ideal gas partial derivative term w.r.t. temperature term
  Real gamma_pie2_r = 0.0;                                    //residual partial derivative term w.r.t. pressure term
  Real gamma_tau2_r = 0.0;                                    //residual partial derivative term w.r.t. temperature term
  Real pie2;                                                  //shifted value of pressure, =p/p*
  Real tau2;                                                  //shifted value of temperature, =T*/T
  Real rt2;                                                   //gas constant * temperature, RT from ideal gas law
  Real pstar2 = 1.0e6;                                        //pressure shifting term, p*
  Real tstar2 = 540.e0;                                       //temperature shifting term, T*
  Real intern_energy2;                                        //*formerly u
  Real enth2_x;                                               //intermediate itteration enthalpy variable *formerly hx
  Real n2_0[9] = { -0.96927686500217e1, 0.10086655968018e2,-0.56087911283020e-2,
                   0.71452738081455e-1, -0.40710498223928e0, 0.14240819171444e1,
                   -0.43839511319450e1,  -0.28408632460772e0, 0.21268463753307e-1 }; //n coefficients of region 2 gibbs eq. ideal gas term
  Real J2_0[9] = { 0, 1, -5, -4, -3, -2, -1, 2, 3 };          //J coefficients of region 2 gibbs eq. ideal gas term
  Real n2_r[43] = { -0.17731742473213e-2, -0.17834862292358e-1,-0.45996013696365e-1,
                    -0.57581259083432e-1, -0.50325278727930e-1, -0.33032641670203e-4,
                    -0.18948987516315e-3, -0.39392777243355e-2, -0.43797295650573e-1,
                    -0.26674547914087e-4, 0.20481737692309e-7, 0.43870667284435e-6,
                    -0.32277677238570e-4, -0.15033924542148e-2, -0.40668253562649e-1,
                    -0.78847309559367e-9, 0.12790717852285e-7, 0.48225372718507e-6,
                    0.22922076337661e-5, -0.16714766451061e-10, -0.21171472321355e-2,
                    -0.23895741934104e2, -0.59059564324270e-17,-0.12621808899101e-5,
                    -0.38946842435739e-1,  0.11256211360459e-10, -0.82311340897998e1,
                    0.19809712802088e-7, 0.10406965210174e-18,-0.10234747095929e-12,
                    -0.10018179379511e-8, -0.80882908646985e-10, 0.10693031879409e0,
                    -0.33662250574171e0, 0.89185845355421e-24, 0.30629316876232e-12,
                    -0.42002467698208e-5, -0.59056029685639e-25, 0.37826947613457e-5,
                    -0.12768608934681e-14, 0.73087610595061e-28, 0.55414715350778e-16,
                    -0.94369707241210e-6 };                                 //n coefficients of region 2 gibbs residual term
  Real J2_r[43] = { 0, 1, 2, 3, 6, 1, 2, 4, 7, 36, 0, 1,
                    3, 6, 35, 1, 2, 3, 7, 3, 16, 35, 0,
                    11, 25, 8, 36, 13, 4, 10, 14, 29, 50,
                    57, 20, 35, 48, 21, 53, 39, 26, 40, 58 };               //J coefficients of region 2 gibbs eq. residual term
  Real I2_r[43] = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3,
                    3, 3, 3, 4, 4, 4, 5, 6, 6, 6, 7, 7,
                    7, 8, 8, 9, 10, 10, 10, 16, 16, 18,
                    20, 20, 20, 21, 22, 23, 24, 24, 24 };                   //I coefficients of region 2 gibbs eq. residual term
  Real dif;


  //Obtain an better initial guess for the output temp.
    //RKP edit August 2013, change the intial guess for temperature to be passed in from the application
    //itr_temp1 = temp_sat - 1.e-6;  //RKP commented out

  if (temp_in < 273.13)
    itr_temp1 = temp_sat - 1.e-6;  //in case temp_in is too low (i.e., below freezing), start at a resaonable point
  else
    itr_temp1 = temp_in;


  //Newton iterations of steam density and enthalpy calculations for region 2
  //(formerly supst function)
  //input - init_temp_guess and press_in
  //output - temp2, enth2, dens2

  while (itr < 151)                                           //exits if Newton iteration does not converge within 150 times
  {
    ////Part 1 (first itteration):

    //Initialize variables
    pie2 = press_in / pstar2;
    tau2 = tstar2 / itr_temp1;
    enth2 = 0.0;
    intern_energy2 = 0.0;
    dens2 = 0.0;
    rt2 = 0.0;
    gamma_tau2_0 = 0.0;
    gamma_pie2_0 = 0.0;
    gamma_tau2_r = 0.0;
    gamma_pie2_r = 0.0;
    gamma_pie2 = 0.0;
    gamma_tau2 = 0.0;

    //gibbs ideal gas terms for region 2
    gamma_pie2_0 = pow(pie2 , -1);

    for (int i=0; i<9; i++)
    {
      gamma_tau2_0 = gamma_tau2_0 + n2_0[i] * J2_0[i] * pow(tau2, (J2_0[i] - 1));
    }

    //gibbs residual terms for region 2
    for (int i=0; i<43; i++)
    {
      gamma_pie2_r = gamma_pie2_r + n2_r[i] * I2_r[i] * pow(pie2 , (I2_r[i] - 1)) * pow((tau2 - 0.5) , J2_r[i]);
      gamma_tau2_r = gamma_tau2_r + n2_r[i] * J2_r[i] * pow(pie2 , I2_r[i]) * pow((tau2 - 0.5) , (J2_r[i] - 1));
    }

    //gibbs free energy terms (ideal gas + residual)
    gamma_pie2 = gamma_pie2_0 + gamma_pie2_r;
    gamma_tau2 = gamma_tau2_0 + gamma_tau2_r;

    rt2 = rconst * itr_temp1;
    dens2 = pstar2 / (rt2 * gamma_pie2);
    intern_energy2 = rt2 * (tau2 * gamma_tau2 - pie2 * gamma_pie2);
    enth2 = intern_energy2 + press_in / dens2;


    dif = std::abs(enth2 - enth_in);


    if (dif <= 1.0e-8)
    {
      break;
    }

    itr_temp2 = itr_temp1 + dt;

    ////Part 2 (second itteration):

    //Re-initialize variables
    pie2 = press_in / pstar2;
    tau2 = tstar2 / itr_temp2;
    intern_energy2 = 0.0;
    dens2 = 0.0;
    rt2 = 0.0;
    gamma_tau2_0 = 0.0;
    gamma_pie2_0 = 0.0;
    gamma_tau2_r = 0.0;
    gamma_pie2_r = 0.0;
    gamma_pie2 = 0.0;
    gamma_tau2 = 0.0;

    //gibbs ideal gas terms for region 2
    gamma_pie2_0 = pow(pie2 , -1);

    for (int i=0; i<9; i++)
    {
      gamma_tau2_0 = gamma_tau2_0 + n2_0[i] * J2_0[i] * pow(tau2, (J2_0[i] - 1));
    }

    //gibbs residual terms for region 2
    for (int i=0; i<43; i++)
    {
      gamma_pie2_r = gamma_pie2_r + n2_r[i] * I2_r[i] * pow(pie2 , (I2_r[i] - 1)) * pow((tau2 - 0.5) , J2_r[i]);
      gamma_tau2_r = gamma_tau2_r + n2_r[i] * J2_r[i] * pow(pie2 , I2_r[i]) * pow((tau2 - 0.5) , (J2_r[i] - 1));
    }

    //gibbs free energy terms (ideal gas + residual)
    gamma_pie2 = gamma_pie2_0 + gamma_pie2_r;
    gamma_tau2 = gamma_tau2_0 + gamma_tau2_r;

    rt2 = rconst * itr_temp2;
    dens2 = pstar2 / (rt2 * gamma_pie2);
    intern_energy2 = rt2 * (tau2 * gamma_tau2 - pie2 * gamma_pie2);
    enth2_x = intern_energy2 + press_in / dens2;
    itr_temp1 = itr_temp1 + (enth_in - enth2) * dt / (enth2_x - enth2);


    itr = itr + 1;

    if (itr > 150)
    {

      //Moose::out << "steam_eq_of_state: Newton iteration does not converge within 150 times. STOP." << std::endl;
      break;

    }
  }

  temp2 = itr_temp1;

  return (0);

}

Real WaterSteamEOS::viscosity (Real density, Real temp, Real& viscosity) const
{
  //Calculates dynamic viscosity of water or steam, given the density
  //and temperature, using the IAPWS industrial formulation 2008.
  //Critical enhancement of viscosity near the critical point is not
  //included.

  Real del;                                                       //shifted value of density, =d/d*
  Real tau;                                                       //shifted value of temperature, =T/T*
  Real dstar = 322.0e0;                                           //density shifting term, d*
  Real tstar = 647.096e0;                                         //temperature shifting term, T*
  Real mustar = 1.00e-6;                                          //viscosity shifting term, mu*
  Real mu0;                                                       //first viscosity term due to dilute-gas limit
  Real mu1;                                                       //second viscosity term due to finite density
  Real summation0 = 0.0;                                          //first summation for dilute-gas limit
  Real summation1 = 0.0;                                          //second summation for finite density
  int I[21] = {0,1,2,3,0,1,2,3,5,0,1,2,3,4,0,1,0,3,4,3,5};        //I exponents of summation 1 term
  int J[21] = {0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,4,4,5,6,6};        //J exponents of summation 1 term
  Real H0[4] = {1.67752e0, 2.20462e0, 0.6366564e0, -0.241605e0};  //H ceofficients for viscosity and summation 0 term
  Real H1[21] = {5.20094e-1, 8.50895e-2, -1.08374e0, -2.89555e-1, 2.22531e-1,
                 9.99115e-1, 1.88797e0, 1.26613e0, 1.20573e-1, -2.81378e-1,
                 -9.06851e-1, -7.72479e-1, -4.89837e-1, -2.57040e-1, 1.61913e-1,
                 2.57399e-1, -3.25372e-2, 6.98452e-2, 8.72102e-3, -4.35673e-3,
                 -5.93264e-4};                                               //H coefficients for viscosity and summation 1 term

  tau = temp / tstar;
  del = density / dstar;

  //Viscosity in dilute-gas limit:
  for (int i=0; i<4; i++)
  {
    summation0 = summation0 + H0[i] / pow(tau, i);
  }

  mu0 = 100.e0 * sqrt(tau) / summation0;

  //Contribution due to finite density:
  for (int i=0; i<21; i++)
  {
    summation1 = summation1 + pow((1/tau - 1) , I[i]) * H1[i] * pow((del - 1) , J[i]);
  }

  mu1 = exp(del * summation1);

  viscosity = mu0 * mu1 * mustar;

  return (viscosity);

}

Real WaterSteamEOS::waterEquationOfStatePT (Real press_in, Real temp, Real& enth_water, Real& dens_water) const
{
  //Variables:
  Real rconst= 0.461526e3;                                        //Gas constant
  Real gamma_pie = 0.0;                                           //partial derivative w.r.t. pressure term for gibbs eq.
  Real gamma_tau = 0.0;                                           //partial derivative w.r.t. temperature term for gibbs. eq.
  Real pie;                                                       //shifted value of pressure, =p/p*
  Real tau;                                                       //shifted value of temperature, =T*/T
  Real rt;                                                        //gas constant * temperature, RT from ideal gas law
  Real pstar = 16.53e6;                                           //pressure shifting term, p*
  Real tstar = 1386.e0;                                           //temperature shifting term, T*
  Real intern_energy_water;                                           //*formerly u
  Real n[34] = { 0.14632971213167e0, -0.84548187169114e0, -0.37563603672040e1,
                 0.33855169168385e1, -0.95791963387872e0, 0.15772038513228e0,
                 -0.16616417199501e-1,0.81214629983568e-3, 0.28319080123804e-3,
                 -0.60706301565874e-3, -0.18990068218419e-1,-0.32529748770505e-1,
                 -0.21841717175414e-1, -0.52838357969930e-4, -0.47184321073267e-3,
                 -0.30001780793026e-3, 0.47661393906987e-4, -0.44141845330846e-5,
                 -0.72694996297594e-15,-0.31679644845054e-4, -0.28270797985312e-5,
                 -0.85205128120103e-9, -0.22425281908000e-5,-0.65171222895601e-6,
                 -0.14341729937924e-12, -0.40516996860117e-6, -0.12734301741641e-8,
                 -0.17424871230634e-9, -0.68762131295531e-18, 0.14478307828521e-19,
                 0.26335781662795e-22,-0.11947622640071e-22, 0.18228094581404e-23,
                 -0.93537087292458e-25 };                                    //n coefficients of region 1 gibbs eq.
  Real I[34] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2,
                 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29,
                 30, 31, 32 };                                               //I coefficients of region 1 gibbs eq.
  Real J[34] = { -2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3,
                 0, 1, 3, 17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29,
                 -31, -38, -39, -40, -41 };                                  //J coefficients of region 1 gibbs eq.

  //Obtain initial conditions:
  pie = press_in / pstar;
  tau = tstar / temp;

  //gibbs free energy terms for resion 1
  for (int i=0; i<34; i++)
  {
    gamma_pie = gamma_pie - n[i] * I[i] * pow((7.1 - pie),(I[i] - 1)) * pow((tau - 1.222),(J[i]));
    gamma_tau = gamma_tau + n[i] * J[i] * pow((7.1 - pie),(I[i])) * pow((tau - 1.222),(J[i] - 1));
  }

  rt = rconst * temp;
  dens_water = pstar / (rt * gamma_pie);
  intern_energy_water = rt * (tau * gamma_tau - pie * gamma_pie);
  enth_water = intern_energy_water + press_in / dens_water;

  return (0);
}

Real WaterSteamEOS::steamEquationOfStatePT (Real press_in, Real temp, Real& enth_steam, Real& dens_steam) const
{
  //Variables:
  Real rconst = 0.461526e3;                                       //Gass constant
  Real gamma_pie = 0.0;                                           //total partial derivative w.r.t. pressure term
  // = gamma_pie_0 + gamma_pie_r
  Real gamma_tau = 0.0;                                           //total partial derivative w.r.t. temperature term
  // = gamma_tau_0 + gamma_tau_r
  Real gamma_pie_0 = 0.0;                                         //ideal gas partial derivative term w.r.t. pressure term
  Real gamma_tau_0 = 0.0;                                         //ideal gas partial derivative term w.r.t. temperature term
  Real gamma_pie_r = 0.0;                                         //residual partial derivative term w.r.t. pressure term
  Real gamma_tau_r = 0.0;                                         //residual partial derivative term w.r.t. temperature term
  Real pie;                                                       //shifted value of pressure, =p/p*
  Real tau;                                                       //shifted value of temperature, =T*/T
  Real rt;                                                        //gas constant * temperature, RT from ideal gas law
  Real pstar = 1.0e6;                                             //pressure shifting term, p*
  Real tstar = 540.e0;                                            //temperature shifting term, T*
  Real intern_energy_steam;                                           //*formerly u
  Real n_0[9] = { -0.96927686500217e1, 0.10086655968018e2,-0.56087911283020e-2,
                  0.71452738081455e-1, -0.40710498223928e0, 0.14240819171444e1,
                  -0.43839511319450e1,  -0.28408632460772e0, 0.21268463753307e-1 }; //n coefficietns of region 2 gibbs ideal gas term
  Real J_0[9] = { 0, 1, -5, -4, -3, -2, -1, 2, 3 };               //J coefficients of region 2 gibbs ideal gas term
  Real n_r[43] = { -0.17731742473213e-2, -0.17834862292358e-1,-0.45996013696365e-1,
                   -0.57581259083432e-1, -0.50325278727930e-1, -0.33032641670203e-4,
                   -0.18948987516315e-3, -0.39392777243355e-2, -0.43797295650573e-1,
                   -0.26674547914087e-4, 0.20481737692309e-7, 0.43870667284435e-6,
                   -0.32277677238570e-4, -0.15033924542148e-2, -0.40668253562649e-1,
                   -0.78847309559367e-9, 0.12790717852285e-7, 0.48225372718507e-6,
                   0.22922076337661e-5, -0.16714766451061e-10, -0.21171472321355e-2,
                   -0.23895741934104e2, -0.59059564324270e-17,-0.12621808899101e-5,
                   -0.38946842435739e-1,  0.11256211360459e-10, -0.82311340897998e1,
                   0.19809712802088e-7, 0.10406965210174e-18,-0.10234747095929e-12,
                   -0.10018179379511e-8, -0.80882908646985e-10, 0.10693031879409e0,
                   -0.33662250574171e0, 0.89185845355421e-24, 0.30629316876232e-12,
                   -0.42002467698208e-5, -0.59056029685639e-25, 0.37826947613457e-5,
                   -0.12768608934681e-14, 0.73087610595061e-28, 0.55414715350778e-16,
                   -0.94369707241210e-6 };                                     //n coefficients of region 2 gibbs residual term
  Real J_r[43] = { 0, 1, 2, 3, 6, 1, 2, 4, 7, 36, 0, 1, 3, 6, 35, 1, 2, 3, 7, 3, 16,
                   35, 0, 11, 25, 8, 36, 13, 4, 10, 14, 29, 50, 57, 20, 35, 48, 21,
                   53, 39, 26, 40, 58 };                                       //J coefficients of region 2 gibbs residual term
  Real I_r[43] = { 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 5, 6, 6, 6,
                   7, 7, 7, 8, 8, 9, 10, 10, 10, 16, 16, 18, 20, 20, 20, 21, 22, 23,
                   24, 24, 24 };                                               //I coefficients of region 2 gibbs residual term

  //Obtain initial conditions:
  pie = press_in / pstar;
  tau = tstar / temp;

  //gibbs ideal gas terms for region 2
  gamma_pie_0 = pow(pie, -1);

  for (int i=0; i<9; i++)
  {
    gamma_tau_0 = gamma_tau_0 + n_0[i] * J_0[i] * pow(tau, (J_0[i] - 1));
  }

  //gibbs residual terms for region 2
  for (int i=0; i<43; i++)
  {
    gamma_pie_r = gamma_pie_r + n_r[i] * I_r[i] * pow(pie, (I_r[i] - 1)) * pow((tau - 0.5) , J_r[i]);
    gamma_tau_r = gamma_tau_r + n_r[i] * J_r[i] * pow(pie, I_r[i]) * pow((tau - 0.5) , (J_r[i] - 1));
  }

  //gibbs free energy terms (ideal gas + residual)
  gamma_pie = gamma_pie_0 + gamma_pie_r;
  gamma_tau = gamma_tau_0 + gamma_tau_r;

  rt = rconst * temp;
  dens_steam = pstar / (rt * gamma_pie);
  intern_energy_steam = rt * (tau * gamma_tau - pie * gamma_pie);
  enth_steam = intern_energy_steam + press_in / dens_steam;

  return (0);
}



//Main functions (one for non-derivatives, one for derivatives):

//WaterSteamEOS: phase_determ, water_eq_of_state, steam_eq_of_state, and viscosity functions are callsed whithin the bellow function.
//This allows for more orginization and the flexibility to call each of these subfunctions without having to call the main function.
//Call this function if the derivatives of the EOS properties w.r.t. pressure and enthalpy ARE NOT needed.
Real WaterSteamEOS::waterAndSteamEquationOfStatePropertiesPH (Real enth_in, Real press_in, Real temp_in, Real& phase, Real& temp_out, Real& temp_sat, Real& sat_fraction_out, Real& dens_out, Real& dens_water_out, Real& dens_steam_out, Real& enth_water_out, Real& enth_steam_out, Real& visc_water_out, Real& visc_steam_out, Real& del_press, Real& del_enth) const
{
  /////VARIABLES:
  Real visc1, visc2/*, visc3*/;                   //output - viscosity, 1 = comp. water, 2 = steam, 3 = sat. mix.
  Real dens1, dens2/*, dens3*/;                   //output - density, 1 = comp. water, 2 = steam, 3 = sat. mix.
  Real enth1, enth2/*, enth3*/;                   //output - enthalpy, 1 = comp. water, 2 = steam, 3 = sat. mix
  Real temp1, temp2;                          //*formerly T
  Real enth_water;                            //*formerly hw
  Real enth_steam;                            //*formerly hg
  Real dens_water;                            //*formerly dw
  Real dens_steam;                            //*formerly dg
//  Real visc_out;
  Real visc_water;
  Real visc_steam;
  //int phase;                                  //output of phase_determ func tion - denotes phase of water *formerly iphase
  //[1] = compress. water, [2] = superheat. steam, [3] = satur. mixture


  /////Part I - Determining Phase of Water (compressed water, steam, or saturated mixture):
  //Function calls
  phaseDetermine (enth_in, press_in, phase, temp_sat, enth_water, enth_steam, dens_water, dens_steam);
  //inputs - enthalpy, pressure
  //outputs - phase ([1], [2], or [3]),
  //          saturation temp,
  //          enthalpy and density of saturated liquid,
  //          enthalpy and density of saturated steam

  /////Part II - Running Equations of State:
  if (phase == 1)                                                     //if water is in the compressed water phase it
    //will run this loop
  {
    //Function calls
    waterEquationOfStatePH (enth_in, press_in, temp_in, temp_sat, temp1, dens1, enth1);
    //inputs - enthalpy, pressure, saturation temp
    //outputs - temperature, density, and enthalpy
    //Performs Newton itterations to find the temperature
    //Stops when enthalpy is within 1e-15 of refrence
    //enthalpy or >150 itterations

    viscosity (dens1, temp1, visc1);                                //inputs - density, temp from water_eq_of_state func.
    //output - viscosity of comp. water

    //outputs to falcon for non-derivatives:
    temp_out = temp1;                                              //*T
    sat_fraction_out = 1.0;                                        //*sw
    dens_water_out = dens1;                                        //*Denw
    dens_steam_out = 1e-15;                                       //*Dens
    enth_water_out = enth_in;                                      //*hw
    enth_steam_out = 0.0;                                          //*hs or hg
    dens_out = dens1;                                              //*Den
    visc_water_out = visc1;
    visc_steam_out = 1e-15;
//    visc_out = visc1;                                              //*visc
    del_press = 0.1;
    del_enth = -0.1;
  }
  else if (phase == 2)                                                //if water is in the steam phase it will run this loop
  {
    //Function calls
    steamEquationOfStatePH (enth_in, press_in, temp_in, temp_sat, temp2, dens2, enth2);
    //inputs - enthalpy, pressure, saturation temp
    //outputs - temperature, density, and enthalpy
    //Performs Newton itterations to find the temperature
    //Stops when enthalpy is within 1e-15 of refrence
    //enthalpy or >150 itterations

    viscosity (dens2, temp2, visc2);                                //inputs - density, temp from steam_eq_of_state func.
    //output - viscosity of steam

    //outputs to falcon for non-derivatives:
    temp_out = temp2;                                              //*T
    sat_fraction_out = 0.0;                                        //*sw
    dens_water_out = 1e-15;                                       //*Denw
    dens_steam_out = dens2;                                        //*Dens
    enth_water_out = 0.0;                                          //*hw
    enth_steam_out = enth_in;                                      //*hs or hg
    dens_out = dens2;                                              //*Den
    visc_water_out = 1e-15;
    visc_steam_out = visc2;
//    visc_out = visc2;                                              //*visc
    del_press = -0.1;
    del_enth = 0.1;
  }
  else if (phase == 3)                                                //if water is a saturated mixture it will run this loop
  {
    //Function calls
    viscosity (dens_water, temp_sat, visc_water);                  //inputs - density of water at sat. temp., saturation
    //temp from phase_determ func.
    //outputs - viscosity of saturated liquid
    viscosity (dens_steam, temp_sat, visc_steam);                  //inputs - density of steam at sat. temp., saturation
    //temp from phase_determ func.
    //outputs - viscosity of saturated steam

    //outputs to falcon for non-derivatives:
    temp_out = temp_sat;                                           //*T
    sat_fraction_out = 1.e0 / (1.e0 - dens_water / dens_steam * (enth_water - enth_in)/ (enth_steam - enth_in)); //*sw
    dens_water_out = dens_water;                                   //*Denw
    dens_steam_out = dens_steam;                                   //*Dens
    enth_water_out = enth_water;                                   //*hw
    enth_steam_out = enth_steam;                                   //*hs or hg
    dens_out = sat_fraction_out * dens_water + (1.e0 - sat_fraction_out) * dens_steam;  //*Den
    visc_water_out = visc_water;
    visc_steam_out = visc_steam;
//    visc_out = sat_fraction_out * visc_water + (1.e0 - sat_fraction_out) * visc_steam;  //*visc
    del_press = 0.1;
    del_enth = 0.1;
  }
  else
  { }
  return (0);

}


//WaterSteamEOS: Equations_of_State_Properties, phase_determ, water_eq_of_state, steam_eq_of_state, water_EOS__deriv_init_values,
//and steam_EOS_deriv_init_values are called within the bellow funtion.  This allows for more organization and flexibility.
//Call this function if the derivatives of the EOS properties w.r.t. pressure and enthalpy ARE needed.
Real WaterSteamEOS::waterAndSteamEquationOfStatePropertiesWithDerivativesPH (Real enth_in, Real press_in, Real temp_in, Real& temp_out, Real& sat_fraction_out, Real& dens_out, Real& dens_water_out, Real& dens_steam_out, Real& enth_water_out, Real& enth_steam_out, Real& visc_water_out, Real& visc_steam_out, Real& d_enth_water_d_press, Real& d_enth_steam_d_press, Real& d_dens_d_press, Real& d_temp_d_press, Real& d_enth_water_d_enth, Real& d_enth_steam_d_enth, Real& d_dens_d_enth, Real& d_temp_d_enth, Real& d_sat_fraction_d_enth) const
{
  //Variables
  //new pressure and enthalpy values shifted by del_press and del_enth:
  Real new_press;                              //*formerly p+delp
  Real new_enth;                               //*formerly h+delh
  Real temp_sat;                              //saturation temperature, *formerly Tsat
  //inital values for derivative calulations:
  Real temp_0;                                 //*formerly t0 - inital value of temperature
  Real enth_water_0;                           //*formerly hw0 - initial value of water enthalpy
  Real enth_steam_0;                           //*formerly hg0 - initial value of steam enthalpy
  Real dens_0;                                 //*formerly Den - inital value of density
  Real dens_water_0;                           //*formerly dw0 - initial value of water density
  Real dens_steam_0;                           //*formerly dg0 - initial value of steam density
  Real sat_fraction_0;                         //*formerly sw0 - initial value of saturation fraction
  //final values for new pressure value (new_press) for derivative calulations:
  Real temp_1p;                                //*formerly t1 - final value of temp after pressure shift
  Real enth_water_1p;                          //*formerly dw1 - final value of water enthalpy after pressure shift
  Real enth_steam_1p;                          //*formerly hg1 - final value of steam enthalpy after pressure shift
  Real dens_1p;                                //*formerly d1 - final value of density after pressure shift
  Real dens_water_1p;                          //*formerly dw1 - final value of water density after pressure shift
  Real dens_steam_1p;                          //*formerly dg1 - final value of steam density after pressure shift
  Real sat_fraction_1p;                        //*formerly sw1 - final value of saturation fraction after pressure shift
  //final values for new enthalpy value (new_enth) forderivative calculations:
  Real temp_1h;                                //*formerly t1 - final value of temp after enthalpy shift
  Real enth_water_1h;                          //*formerly hw1 - final value of water enthalpy after enthalpy shift
  Real enth_steam_1h;                          //*formerly hg1 - final value of steam enthalpy after enthalpy shift
  Real dens_1h;                                //*formerly d1 - final value of density after enthalpy shift
//  Real dens_water_1h;                          //*formerly dw1 - final value of water density after enthalpy shift
//  Real dens_steam_1h;                          //*formerly dg1 - final value of steam density after enthalpy figt
  Real sat_fraction_1h;                        //*formerly sw1 - final value of saturation fraction after enthalpy shift
  Real phase;                                  //current phase of fluid (water, steam, saturated mix)
  Real del_press, del_enth;                    //delta increments added to enthalpy and pressure to determine derivatives


  //Obtain non-derivative properties:
  waterAndSteamEquationOfStatePropertiesPH (enth_in, press_in, temp_in, phase, temp_out, temp_sat, sat_fraction_out, dens_out, dens_water_out, dens_steam_out, enth_water_out, enth_steam_out, visc_water_out, visc_steam_out, del_press, del_enth);
  //viscosity terms outputs from this function are not used

  //New-incremented pressure and enthalpy:
  new_press = press_in + del_press;
  new_enth = enth_in + del_enth;
  temp_0 = temp_out;

  if (phase == 1)                                                     //if water is in a compressed water phase it will run this loop
  {
    //Function calls
    waterEquationOfStatePT (press_in, temp_0, enth_water_0, dens_0);
    //Determining initial enthalpy and density for derivative
    //calculations
    //inputs - press_in, temp_0
    //outputs - enth_water_0, dens_0

    waterEquationOfStatePH (enth_in, new_press, temp_in, temp_sat, temp_1p, dens_1p, enth_water_1p);
    //Determining final enthalpy and density w.r.t new 'incremented'
    //pressure value (new_press = press_in + del_press)
    //inputs - enth_in, new_press, temp_sat
    //outputs - temp_1p, dens_1p, enth_water_1p

    waterEquationOfStatePH (new_enth, press_in, temp_in, temp_sat, temp_1h, dens_1h, enth_water_1h);
    //Determining final enthalpy and density w.r.t new 'incremented'
    //enthalpy value (new_enth = enth_in + del_enth)
    //inputs - new_enth, press_in, temp_sat
    //outputs - temp_1h, dens_1h, enth_water_1h

    //outputs to falcon - derivatives with respect to pressure
    d_enth_water_d_press = ((enth_water_1p - enth_water_0) / del_press);//*dhwdp
    //_d_enth_water_d_press = 1e-9;
    d_enth_steam_d_press = 0.0;                                    //*dhshp
    d_dens_d_press = (dens_1p - dens_0) / del_press;               //*dDendp
    d_temp_d_press = (temp_1p - temp_0) / del_press;               //*dTdp


    //outputs to falcon - derivatives with respect to enthalpy
    d_enth_water_d_enth = ((enth_water_1h - enth_water_0) / del_enth); //*dhwdh
    //_d_enth_water_d_enth = 0.0;
    d_enth_steam_d_enth = 0.0;                                     //*dhsdh
    d_dens_d_enth = ((dens_1h - dens_0) / del_enth);               //*dDendh
    d_temp_d_enth = ((temp_1h - temp_0) / del_enth);               //*dTdh
    d_sat_fraction_d_enth = 0.0;                                   //*dswdh

  }

  else if (phase == 2)                                             //if water is in a steam phase it will run this loop
  {
    //Function calls
    steamEquationOfStatePT (press_in, temp_0, enth_steam_0, dens_0);
    //Determining initial enthalpy and density for derivative
    //calculations
    //inputs - press_in, temp_0
    //outputs - enth_steam_0, dens_0

    steamEquationOfStatePH (enth_in, new_press, temp_in, temp_sat, temp_1p, dens_1p, enth_steam_1p);
    //Determining final enthalpy and density w.r.t new 'incremented'
    //pressure value (new_press = press_in + del_press)
    //inputs - enth_in, new_press, temp_sat
    //outputs - temp_1p, dens_1p, enth_steam_1p

    steamEquationOfStatePH (new_enth, press_in, temp_in, temp_sat, temp_1h, dens_1h, enth_steam_1h);
    //Determining final enthalpy and density w.r.t new 'incremented'
    //enthalpy value (new_enth = enth_in + del_enth)
    //inputs - new_enth, press_in, temp_sat
    //outputs - temp_1h, dens_1h, enth_steam_1h

    //outputs to falcon - derivatives with respect to pressure
    d_enth_water_d_press = 0.0;                                    //*dhwdp
    d_enth_steam_d_press = ((enth_steam_1p - enth_steam_0) / del_press); //*dhsdp
    d_dens_d_press = (dens_1p - dens_0) / del_press;               //*dDendp
    d_temp_d_press = (temp_1p - temp_0) / del_press;               //*dTdp

    //outputs to falcon - derivatives with respect to enthalpy
    d_enth_water_d_enth = 0.0;                                     //*dhwdh
    d_enth_steam_d_enth = 1.e0;                                    //*dhsdh
    d_dens_d_enth = ((dens_1h - dens_0) / del_enth);               //*dDendh
    d_temp_d_enth = ((temp_1h - temp_0) / del_enth);               //*dTdh
    d_sat_fraction_d_enth = 0.0;                                   //*dswdh

  }
  else if (phase == 3)                                                //if water is in a saturated mixture phase it will run this loop
  {
    //Reassigning calcuated variables from Equations_of_State_Properties function to new variables for phase 3 derivative loop
    dens_water_0 = dens_water_out;
    enth_water_0 = enth_water_out;
    sat_fraction_0 = sat_fraction_out;
    dens_steam_0 = dens_steam_out;
    enth_steam_0 = enth_steam_out;
    dens_0 = dens_out;

    //Function calls
    phaseDetermine (enth_in, new_press, phase, temp_1p, enth_water_1p, enth_steam_1p, dens_water_1p, dens_steam_1p);
    //Determining final enthalpy, density, and saturation fraction
    //values with respect to new 'incremented' pressure value
    //(new_press = press_in + del_press)
    //inputs - enth_in, new_press
    //outputs - phase, temp_1p, enth_water_1p, enth_steam_1p,
    //          dens_water_1p, dens_steam_1p

    //Some calculations to determine final values of saturation fraction and density
    sat_fraction_1p = 1.e0 / (1.e0 - dens_water_1p / dens_steam_1p * (enth_water_1p - enth_in) / (enth_steam_1p - enth_in));
    sat_fraction_1h = 1.e0 / (1.e0 - dens_water_0 / dens_steam_0 * (enth_water_0 - new_enth) / (enth_steam_0 - new_enth));
    dens_1p = sat_fraction_1p * dens_water_1p + (1.e0 - sat_fraction_1p) * dens_steam_1p;
    dens_1h = sat_fraction_1h * dens_water_0 + (1.e0 - sat_fraction_1h) * dens_steam_0;

    //outputs to falcon - derivatives with repect to pressure
    d_enth_water_d_press = ((enth_water_1p - enth_water_0) / del_press);     //*dhwdp
    d_enth_steam_d_press = ((enth_steam_1p - enth_steam_0) / del_press);     //*dhsdp
    d_dens_d_press = (dens_1p - dens_0) / del_press;               //*dDendp
    d_temp_d_press = (temp_1p - temp_0) / del_press;               //*dTdp

    //outputs to falcon - derivatives with respect to enthalpy
    d_enth_water_d_enth = 0.0;                                     //*dhwdh
    d_enth_steam_d_enth = 0.0;                                     //*dhsdh
    d_dens_d_enth = ((dens_1h - dens_0) / del_enth);               //*dDendh
    d_temp_d_enth = 0.0;                                           //*dTdh
    d_sat_fraction_d_enth = ((sat_fraction_1h - sat_fraction_0) / del_enth); //*dswdh
  }
  return (0);
}
