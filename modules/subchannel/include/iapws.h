#ifndef IAPWS_H
#define IAPWS_H

#include <cmath>

namespace iapws {

//==============================================================================
// Region 1 forward equations.
//==============================================================================

// Declare constants for the region 1 forward equations.
const double R = 0.461526;  // kJ / kg / K
const double _n1f[] = {
  0.14632971213167, -0.84548187169114, -0.37563603672040e1,
  0.33855169168385e1, -0.95791963387872, 0.15772038513228,
  -0.16616417199501e-1, 0.81214629983568e-3, 0.28319080123804e-3,
  -0.60706301565874e-3, -0.18990068218419e-1, -0.32529748770505e-1,
  -0.21841717175414e-1, -0.52838357969930e-4, -0.47184321073267e-3,
  -0.30001780793026e-3, 0.47661393906987e-4, -0.44141845330846e-5,
  -0.72694996297594e-15, -0.31679644845054e-4, -0.28270797985312e-5,
  -0.85205128120103e-9, -0.22425281908000e-5, -0.65171222895601e-6,
  -0.14341729937924e-12, -0.40516996860117e-6, -0.12734301741641e-8,
  -0.17424871230634e-9, -0.68762131295531e-18, 0.14478307828521e-19,
  0.26335781662795e-22, -0.11947622640071e-22, 0.18228094581404e-23,
  -0.93537087292458e-25};
const int _I1f[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
  3, 3, 3, 4, 4, 4, 5, 8, 8, 21, 23, 29, 30, 31, 32};
const int _J1f[] = {-2, -1, 0, 1, 2, 3, 4, 5, -9, -7, -1, 0, 1, 3, -3, 0, 1, 3,
  17, -4, 0, 6, -5, -2, 10, -8, -11, -6, -29, -31, -38, -39, -40, -41};

//==============================================================================

inline double
gamma1(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 34; i++)
  {
    out += _n1f[i] * pow(7.1 - pi, _I1f[i]) * pow(tau - 1.222, _J1f[i]);
  }
  return out;
}

//==============================================================================

inline double
gamma1_pi(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 34; i++)
  {
    out -= _n1f[i] * _I1f[i] * pow(7.1 - pi, _I1f[i] - 1)
           * pow(tau - 1.222, _J1f[i]);
  }
  return out;
}

//==============================================================================

inline double
gamma1_tau(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 34; i++)
  {
    out += _n1f[i] * pow(7.1 - pi, _I1f[i]) * _J1f[i]
      * pow(tau - 1.222, _J1f[i] - 1);
  }
  return out;
}

//==============================================================================

inline double
gamma1_tau_tau(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 34; i++)
  {
    out += _n1f[i] * pow(7.1 - pi, _I1f[i]) * _J1f[i] * (_J1f[i] - 1)
      * pow(tau - 1.222, _J1f[i] - 2);
  }
  return out;
}

//==============================================================================
// Region 1 thermodynamic properties.
//==============================================================================

inline double
nu1(double p, double T)
{
  double pi = p / 16.53;  // Dimensionless pressure; p must be in MPa.
  double tau = 1386.0 / T;  // Dimensionless temperature; T must be in K.
  return pi * gamma1_pi(pi, tau) * R * T / (p * 1e3);
}

//==============================================================================

inline double
u1(double p, double T)
{
  double pi = p / 16.53;  // Dimensionless pressure; p must be in MPa.
  double tau = 1386.0 / T;  // Dimensionless temperature; T must be in K.
  return (tau * gamma1_tau(pi, tau) - pi * gamma1_pi(pi, tau)) * R * T;
}

//==============================================================================

inline double
s1(double p, double T)
{
  double pi = p / 16.53;  // Dimensionless pressure; p must be in MPa.
  double tau = 1386.0 / T;  // Dimensionless temperature; T must be in K.
  return (tau * gamma1_tau(pi, tau) - gamma1(pi, tau)) * R;
}

//==============================================================================

inline double
h1(double p, double T)
{
  double pi = p / 16.53;  // Dimensionless pressure; p must be in MPa.
  double tau = 1386.0 / T;  // Dimensionless temperature; T must be in K.
  return tau * gamma1_tau(pi, tau) * R * T;
}

//==============================================================================

inline double
cp1(double p, double T)
{
  double pi = p / 16.53;  // Dimensionless pressure; p must be in MPa.
  double tau = 1386.0 / T;  // Dimensionless temperature; T must be in K.
  return -tau * tau * gamma1_tau_tau(pi, tau) * R;
}

//==============================================================================
// Region 1 backward equations.
//==============================================================================

const double _n1bh[] = {
  -0.23872489924521e3, 0.40421188637945e3, 0.11349746881718e3,
  -0.58457616048039e1, -0.15285482413140e-3, -0.10866707695377e-5,
  -0.13391744872602e2, 0.43211039183559e2, -0.54010067170506e2,
  0.30535892203916e2, -0.65964749423638e1, 0.93965400878363e-2,
  0.11573647505340e-6, -0.25858641282073e-4, -0.40644363084799e-8,
  0.66456186191635e-7, 0.80670734103027e-10, -0.93477771213947e-12,
  0.58265442020601e-14, -0.15020185953503e-16};
const int _I1bh[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5,
  6};
const int _J1bh[] = {0, 1, 2, 6, 22, 32, 0, 1, 2, 3, 4, 10, 32, 10, 32, 10, 32,
  32, 32, 32};

//==============================================================================

inline double
T_from_p_h(double p, double h)
{
  double pi = p;  // p must be in MPa
  double eta = h / 2500.0;  // h must be in kJ / kg
  double theta = 0.0;
  for (int i = 0; i < 20; i++)
  {
    theta += _n1bh[i] * pow(pi, _I1bh[i]) * pow(eta + 1, _J1bh[i]);
  }
  return theta;
}

//==============================================================================
// Region 2 forward equations.
//==============================================================================

const double _n2igf[] = {
  -0.96927686500217e1, 0.10086655968018e2, -0.56087911283020e-2,
  0.71452738081455e-1, -0.40710498223928, 0.14240819171444e1,
  -0.43839511319450e1, -0.28408632460772, 0.21268463753307e-1};
const int _J2igf[] = {0, 1, -5, -4, -3, -2, -1, 2, 3};
const double _n2rf[] = {
  -0.17731742473213e-2, -0.17834862292358e-1, -0.45996013696365e-1,
  -0.57581259083432e-1, -0.50325278727930e-1, -0.33032641670203e-4,
  -0.18948987516315e-3, -0.39392777243355e-2, -0.43797295650573e-1,
  -0.26674547914087e-4, 0.20481737692309e-7, 0.43870667284435e-6,
  -0.32277677238570e-4, -0.15033924542148e-2, -0.40668253562649e-1,
  -0.78847309559367e-9, 0.12790717852285e-7, 0.48225372718507e-6,
  0.22922076337661e-5, -0.16714766451061e-10, -0.21171472321355e-2,
  -0.23895741934104e2, -0.59059564324270e-17, -0.12621808899101e-5,
  -0.38946842435739e-1, 0.11256211360459e-10, -0.82311340897998e1,
  0.19809712802088e-7, 0.10406965210174e-18, -0.10234747095929e-12,
  -0.10018179379511e-8, -0.80882908646985e-10, 0.10693031879409,
  -0.33662250574171, 0.89185845355421e-24, 0.30629316876232e-12,
  -0.42002467698208e-5, -0.59056029685639e-25, 0.37826947613457e-5,
  -0.12768608934681e-14, 0.73087610595061e-28, 0.55414715350778e-16,
  -0.94369707241210e-6};
const int _I2rf[] = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 5, 6,
  6, 6, 7, 7, 7, 8, 8, 9, 10, 10, 10, 16, 16, 18, 20, 20, 20, 21, 22, 23, 24,
  24, 24};
const int _J2rf[] = {0, 1, 2, 3, 6, 1, 2, 4, 7, 36, 0, 1, 3, 6, 35, 1, 2, 3, 7,
  3, 16, 35, 0, 11, 25, 8, 36, 13, 4, 10, 14, 29, 50, 57, 20, 35, 48, 21, 53,
  39, 26, 40, 58};

//==============================================================================

inline double
gamma2ig(double pi, double tau)
{
  double out = log(pi);
  for (int i = 0; i < 9; i++)
  {
    out += _n2igf[i] * pow(tau, _J2igf[i]);
  }
  return out;
}

//==============================================================================

inline double
gamma2r(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 43; i++)
  {
    out += _n2rf[i] * pow(pi, _I2rf[i]) * pow(tau - 0.5, _J2rf[i]);
  }
  return out;
}

//==============================================================================

inline double
gamma2ig_pi(double pi, double tau)
{
  return 1.0 / pi;
}

//==============================================================================

inline double
gamma2r_pi(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 43; i++)
  {
    out += _n2rf[i] * _I2rf[i] * pow(pi, _I2rf[i] - 1)
           * pow(tau - 0.5, _J2rf[i]);
  }
  return out;
}

//==============================================================================

inline double
gamma2ig_tau(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 9; i++)
  {
    out += _n2igf[i] * _J2igf[i] * pow(tau, _J2igf[i] - 1);
  }
  return out;
}

//==============================================================================

inline double
gamma2r_tau(double pi, double tau)
{
  double out = 0.0;
  for (int i = 0; i < 43; i++)
  {
    out += _n2rf[i] * pow(pi, _I2rf[i]) * _J2rf[i]
           * pow(tau - 0.5, _J2rf[i] - 1);
  }
  return out;
}

//==============================================================================
// Region 2 thermodynamic properties.
//==============================================================================

inline double
nu2(double p, double T)
{
  double pi = p;  // Dimensionless pressure; p must be in MPa.
  double tau = 540.0 / T;  // Dimensionless temperature; T must be in K.
  return pi * (gamma2ig_pi(pi, tau) + gamma2r_pi(pi, tau)) * R * T / (p * 1e3);
}

//==============================================================================

inline double
h2(double p, double T)
{
  double pi = p;  // Dimensionless pressure; p must be in MPa.
  double tau = 540.0 / T;  // Dimensionless temperature; T must be in K.
  return tau * (gamma2ig_tau(pi, tau) + gamma2r_tau(pi, tau)) * R * T;
}

//==============================================================================
// Region 4 (saturation line) equations.
//==============================================================================

const double _n4[] = {
  0.11670521452767e4, -0.72421316703206e6, -0.17073846940092e2,
  0.12020824702470e5, -0.32325550322333e7, 0.14915108613530e2,
  -0.48232657361591e4, 0.40511340542057e6, -0.23855557567849,
  0.65017534844798e3};

//==============================================================================

inline double
sat_press(double T)
{
  double theta = T + _n4[8] / (T - _n4[9]);
  double A = pow(theta, 2) + _n4[0] * theta + _n4[1];
  double B = _n4[2] * pow(theta, 2) + _n4[3] * theta + _n4[4];
  double C = _n4[5] * pow(theta, 2) + _n4[6] * theta + _n4[7];
  return pow(2.0 * C / (-B + sqrt(pow(B, 2) - 4 * A * C)), 4);
}

//==============================================================================

inline double
sat_temp(double p)
{
  double beta = pow(p, 0.25);
  double E = pow(beta, 2) + _n4[2] * beta + _n4[5];
  double F = _n4[0] * pow(beta, 2) + _n4[3] * beta + _n4[6];
  double G = _n4[1] * pow(beta, 2) + _n4[4] * beta + _n4[7];
  double D = 2.0 * G / (-F - sqrt(pow(F, 2) - 4 * E * G));
  return 0.5 * (_n4[9] + D - sqrt(pow(_n4[9] + D, 2)
                                  - 4.0 * (_n4[8] + _n4[9] * D)));
}

//==============================================================================
// Viscosity equations.
//==============================================================================

const double _H_i_vals[] = {1.67752, 2.20462, 0.6366564, -0.241605};

inline double
mu_0(double T_bar)
{
  double denominator = 0.0;
  for (int i = 0; i < 4; i++)
  {
    denominator += _H_i_vals[i] / pow(T_bar, i);
  }
  return 100.0 * sqrt(T_bar) / denominator;
}

//==============================================================================

const double _H_ij_vals[] = {
  // H[0, :]
  5.20094e-1,  2.22531e-1, -2.81378e-1, 1.61913e-1, -3.25372e-2, 0.0, 0.0,
  // H[1, :]
  8.50895e-2,  9.99115e-1, -9.06851e-1, 2.57399e-1, 0.0,         0.0, 0.0,
  // H[2, :]
  -1.08374,    1.88797,    -7.72479e-1, 0.0,        0.0,         0.0, 0.0,
  // H[3, :]
  -2.89555e-1, 1.26613,    -4.89837e-1, 0.0,        6.98452e-2,  0.0,
  -4.35673e-3,
  // H[4, :]
  0.0,         0.0,        -2.57040e-1, 0.0,        0.0,         8.72102e-3,
  0.0,
  // H[5, :]
  0.0,         1.20573e-1, 0.0,         0.0,        0.0,         0.0,
  -5.93264e-4};

inline double
mu_1(double T_bar, double rho_bar)
{
  double out = 0.0;
  for (int i = 0; i < 6; i++)
  {
    double coeff = 0.0;
    for (int j = 0; j < 7; j++)
    {
      coeff += _H_ij_vals[7*i + j] * pow(rho_bar - 1, j);
    }
    out += coeff * pow(1.0 / T_bar - 1, i);
  }
  return exp(rho_bar * out);
}

//==============================================================================

inline double
mu(double T, double rho)
{
  double T_bar = T / 647.096;
  double rho_bar = rho / 322.0;
  return mu_0(T_bar) * mu_1(T_bar, rho_bar) * 1e-6;
}

//==============================================================================
// Thermal conductivity equations.
//==============================================================================

const double _L_k_vals[] = {
  2.443221e-3, 1.323095e-2, 6.770357e-3, -3.454586e-3, 4.096266e-4};

inline double
k_0(double T_bar)
{
  double denominator = 0.0;
  for (int k = 0; k < 5; k++)
  {
    denominator += _L_k_vals[k] / pow(T_bar, k);
  }
  return sqrt(T_bar) / denominator;
}

//==============================================================================

const double _L_ij_vals[] = {
  // L[0, :]
  1.60397357, -0.646013523, 0.111443906, 0.102997357, -0.0504123634,
  0.00609859258,
  // L[1, :]
  2.33771842, -2.78843778, 1.53616167, -0.46304551, 0.0832827019,
  -0.00719201245,
  // L[2, :]
  2.19650529, -4.54580785, 3.55777244, -1.40944978, 0.275418278, -0.0205938816,
  // L[3, :]
  -1.21051378, 1.60812989, -0.621178141, 0.0716373224, 0.0, 0.0,
  // L[4, :]
  -2.7203370, 4.57586331, -3.18369245, 1.1168348, -0.19268305, 0.012913842,
  };
//_L_ij_vals = np.zeros((5, 6))

inline double
k_1(double T_bar, double rho_bar)
{
  double out = 0.0;
  for (int i = 0; i < 5; i++)
  {
    double coeff = 0.0;
    for (int j = 0; j < 6; j++)
    {
      coeff += _L_ij_vals[6*i + j] * pow(rho_bar - 1, j);
    }
    out += coeff * pow(1.0 / T_bar - 1, i);
  }
  return exp(rho_bar * out);
}

//==============================================================================

inline double
k(double T, double rho)
{
  double T_bar = T / 647.096;
  double rho_bar = rho / 322.0;
  return k_0(T_bar) * k_1(T_bar, rho_bar) * 1e-3;
}

//==============================================================================
// Surface tension.
//==============================================================================

inline double
surf_tens(double T)
{
  double tau = 1.0 - T / 647.096;
  return 0.2358 * pow(tau, 1.256) * (1.0 - 0.625 * tau);
}

} // namespace iapws
#endif // IAPWS_H
