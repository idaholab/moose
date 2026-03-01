#include "GasLiquidMassTransferAux.h"
#include "GasLiquidMassTransferScalarAux.h"
#include "MathUtils.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("MooseApp", GasLiquidMassTransferScalarAux);

InputParameters
GasLiquidMassTransferScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  // Required params
  params.addRequiredCoupledVar("p", "Pressure [Pa]");
  params.addRequiredCoupledVar("T", "Temperature [K]");
  params.addRequiredCoupledVar("fluid_velocity", "Velocity vector [m/s]");
  params.addRequiredParam<Real>("d", "Pipe diameter [m]");
  params.addRequiredParam<UserObjectName>(
      "fp", "The name of the user object for liquid side fluid properties");
  MooseEnum equation_list("StokesEinstein WilkeChang");
  params.addRequiredParam<MooseEnum>(
      "equation", equation_list, "The equation to use for mass transfer calculation");
  // Required for Stokes-Einstein
  params.addParam<Real>("radius", "Particle radius [m]");
  // Required for Wilke-Chang
  params.addParam<Real>("molar_weight", "molar weight solvent [kg/mol]");
  // Optional
  // Wilke-Change gives a phi value of 1.0 for nonassociated solvents and 2.6 for water.
  params.addParam<Real>("phi", 1.0, "The association parameter for the solute (see Wilke-Chang)");
  params.addParam<Real>("wc", 7.4e-8, "WilkeChang constant");
  params.addParam<Real>("db", 0.023, "Dittus-Boelter equation constant");
  params.addClassDescription("Calculates overall liquid mass transfer coefficient [m/s].");
  return params;
}

GasLiquidMassTransferScalarAux::GasLiquidMassTransferScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pressure(coupledScalarValue("p")),
    _temperature(coupledScalarValue("T")),
    _fluid_velocity(coupledScalarValue("fluid_velocity")),
    _diameter(getParam<Real>("d")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _equation_list(getParam<MooseEnum>("equation").getEnum<Equationlist>()),
    _radius(isParamValid("radius") ? getParam<Real>("radius") : 0.0),
    _mw(isParamValid("molar_weight") ? getParam<Real>("molar_weight") : 0.0),
    _phi(getParam<Real>("phi")),
    _wc(getParam<Real>("wc")),
    _db(getParam<Real>("db"))
{
  switch (_equation_list)
  {
    case Equationlist::WILKECHANG:
    {
      if (!isParamSetByUser("molar_weight"))
        mooseError("Must set the molecular weight of the gas when using WilkeChang");
      break;
    }
    case Equationlist::STOKESEINSTEIN:
    {
      if (!isParamSetByUser("radius"))
        mooseError("Must set particle radius when using StokesEinstein");
      break;
    }
  }
}

Real
GasLiquidMassTransferScalarAux::computeValue()
{
  Real mu = _fp.mu_from_p_T(_pressure[0], _temperature[0]);
  Real rho = _fp.rho_from_p_T(_pressure[0], _temperature[0]);
  Real Diffusivity = 0.0;
  switch (_equation_list)
  {
    case Equationlist::STOKESEINSTEIN:
      Diffusivity =
          GasLiquidMassTransferAux::_kB * _temperature[0] / (6.0 * libMesh::pi * mu * _radius);
      break;

    case Equationlist::WILKECHANG:
      // Equation 5 of:
      // C.R. Wilke, P. Chang, Correlation of diffusion coefficients in dilute solutions, AICHE J.,
      // 1955, 1(2) 264-270 Units of Wilke Chang are g-cm-s
      Real kg_to_g = 1000.0;
      Real m_to_cm = 100.0;
      Real cm_to_m = 1 / m_to_cm;
      Real mu_cgs = mu * kg_to_g / m_to_cm; // g/cm/s = Poise
      Real poise_to_centipoise = 100;
      mu_cgs = mu_cgs * poise_to_centipoise;           // cP
      Real molar_volume = _mw / rho * pow(m_to_cm, 3); // cm3/mol
      Real molar_weight = _mw * kg_to_g;               // g/mol
      Diffusivity = _wc * _temperature[0] * std::sqrt(_phi * molar_weight) /
                    (mu_cgs * std::pow(molar_volume, 0.6));
      Diffusivity = Diffusivity * pow(cm_to_m, 2);
      break;
  }

  Real re = rho * abs(_fluid_velocity[0]) * _diameter / mu;
  Real sc = mu / (rho * Diffusivity);
  Real mtc = _db * std::pow(re, 0.8) * std::pow(sc, 0.4) * Diffusivity / _diameter;
  return mtc;
}
