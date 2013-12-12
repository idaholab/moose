#include <cmath> // std::sinh and std::cosh
#include "RichardsMaterial.h"



template<>
InputParameters validParams<RichardsMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("mat_porosity", "The porosity of the material.  Should be between 0 and 1.  Eg, 0.1");
  params.addRequiredParam<RealTensorValue>("mat_permeability", "The permeability tensor (m^2).");
  params.addRequiredParam<UserObjectName>("relperm_UO", "Name of user object that defines relative permeability");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation as a function of capillary pressure");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation as a function of effective saturation");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<Real>("dens0", "Density of fluid at pressure=0.  Must be positive.  Typical value for water=1000.  This is only used if SUPG is active");
  params.addRequiredParam<Real>("viscosity", "Viscosity of fluid (Pa.s).  Typical value for water is=1E-3");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration (m/s^2) as a vector pointing downwards.  Eg (0,0,-10).   This is only used if SUPG is active");
  params.addRequiredCoupledVar("pressure_variable", "The name of the pressure variable");
  params.addRequiredParam<bool>("SUPG", "Whether to use SUPG for this material");
  params.addParam<Real>("SUPG_pressure", 1E5, "If using SUPG, this parameter controls the strength of the upwinding.  This must be positive.  If you need to track advancing fronts in a problem, then set to less than your expected range of pressures in your unsaturated zone.  Otherwise, set larger, and then minimal upwinding will occur and convergence will typically be good.");
  params.addParam<bool>("linear_shape_fcns", true, "If you are using second-order Lagrange shape functions you need to set this to false.");
  
  return params;
}

RichardsMaterial::RichardsMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),
    _material_por(getParam<Real>("mat_porosity")),
    _material_perm(getParam<RealTensorValue>("mat_permeability")),
    _material_relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO")),
    _material_seff_UO(getUserObject<RichardsSeff>("seff_UO")),
    _material_sat_UO(getUserObject<RichardsSat>("sat_UO")),
    _material_density_UO(getUserObject<RichardsDensity>("density_UO")),
    _material_dens0(getParam<Real>("dens0")),
    _material_viscosity(getParam<Real>("viscosity")),
    _material_gravity(getParam<RealVectorValue>("gravity")),
    _pressure(coupledValue("pressure_variable")),

    _pressure_old(_is_transient ? coupledValueOld("pressure_variable") : _zero),

    // FOLLOWING IS FOR SUPG
    _doing_SUPG(getParam<bool>("SUPG")),
    _SUPG_pressure(getParam<Real>("SUPG_pressure")),
    _grad_p(_doing_SUPG ? coupledGradient("pressure_variable") : _grad_zero),
    _trace_perm(_material_perm.tr()),

    // Grab reference to linear Lagrange finite element object pointer,
    // currently this is always a linear Lagrange element, so this might need to
    // be generalized if we start working with higher-order elements...
    _fe(_subproblem.assembly(_tid).getFE( getParam<bool>("linear_shape_fcns") ? FEType(FIRST, LAGRANGE) : FEType(SECOND, LAGRANGE))),
    
    // Grab references to FE object's mapping data from the _subproblem's FE object
    _dxidx(_fe->get_dxidx()),
    _dxidy(_fe->get_dxidy()),
    _dxidz(_fe->get_dxidz()),
    _detadx(_fe->get_detadx()),
    _detady(_fe->get_detady()),
    _detadz(_fe->get_detadz()),
    _dzetadx(_fe->get_dzetadx()), // Empty in 2D
    _dzetady(_fe->get_dzetady()), // Empty in 2D
    _dzetadz(_fe->get_dzetadz()), // Empty in 2D

    // Declare that this material is going to provide a Real
    // valued property named "porosity", etc, that Kernels can use.
    _porosity(declareProperty<Real>("porosity")),
    _permeability(declareProperty<RealTensorValue>("permeability")),

    _dens0(declareProperty<Real>("dens0")),
    _viscosity(declareProperty<Real>("viscosity")),
    _gravity(declareProperty<RealVectorValue>("gravity")),

    _density_old(declareProperty<Real>("density_old")),

    _density(declareProperty<Real>("density")),
    _ddensity(declareProperty<Real>("ddensity")),
    _d2density(declareProperty<Real>("d2density")),

    _seff_old(declareProperty<Real>("s_eff_old")),

    _seff(declareProperty<Real>("s_eff")),
    _dseff(declareProperty<Real>("ds_eff")),
    _d2seff(declareProperty<Real>("d2s_eff")),

    _sat_old(declareProperty<Real>("sat_old")),

    _sat(declareProperty<Real>("sat")),
    _dsat(declareProperty<Real>("dsat")),
    _d2sat(declareProperty<Real>("d2sat")),

    _rel_perm(declareProperty<Real>("rel_perm")),
    _drel_perm(declareProperty<Real>("drel_perm")),
    _d2rel_perm(declareProperty<Real>("d2rel_perm")),

    _vel_SUPG(declareProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(declareProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(declareProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(declareProperty<RealVectorValue>("tau_prime_SUPG"))

{
  if (_material_por <= 0 || _material_por >= 1)
    mooseError("Porosity set to " << _material_por << " but it must be between 0 and 1");

  if (_doing_SUPG && _SUPG_pressure <= 0)
    mooseError("When using SUPG, SUPG_pressure must be positive.  Yours is " << _SUPG_pressure);
  
}

/*
void
RichardsMaterial::computeQpProperties()
{
  _porosity[_qp] = _material_por;
  _permeability[_qp] = _material_perm;
}
*/

void
RichardsMaterial::computeProperties()
{
  // this gets run for each element
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
    {

      _porosity[qp] = _material_por;
      _permeability[qp] = _material_perm;


      _dens0[qp] = _material_dens0;
      _viscosity[qp] = _material_viscosity;
      _gravity[qp] = _material_gravity;

      _density_old[qp] = _material_density_UO.density(_pressure_old[qp]);
      _density[qp] = _material_density_UO.density(_pressure[qp]);
      _ddensity[qp] = _material_density_UO.ddensity(_pressure[qp]);
      _d2density[qp] = _material_density_UO.d2density(_pressure[qp]);

      _seff_old[qp] = _material_seff_UO.seff(-_pressure_old[qp]);
      _seff[qp] = _material_seff_UO.seff(-_pressure[qp]);
      _dseff[qp] = -_material_seff_UO.dseff(-_pressure[qp]); // minus sign because we want deriv wrt pressure, not pc
      _d2seff[qp] = _material_seff_UO.d2seff(-_pressure[qp]);

      _sat_old[qp] = _material_sat_UO.sat(_seff_old[qp]);
      _sat[qp] = _material_sat_UO.sat(_seff[qp]);
      _dsat[qp] = _material_sat_UO.dsat(_seff[qp])*_dseff[qp];
      _d2sat[qp] = _material_sat_UO.d2sat(_seff[qp])*_dseff[qp] + _material_sat_UO.dsat(_seff[qp])*_d2seff[qp];


      _rel_perm[qp] = _material_relperm_UO.relperm(_seff[qp]);
      _drel_perm[qp] = _material_relperm_UO.drelperm(_seff[qp]);
      _d2rel_perm[qp] = _material_relperm_UO.d2relperm(_seff[qp]);

      _vel_SUPG[qp] = (_doing_SUPG ? velSUPG(qp) : 0*velSUPG(qp)); // ZERO properly
      _vel_prime_SUPG[qp] = (_doing_SUPG ? velPrimeSUPG(qp) : 0*velPrimeSUPG(qp)); // ZERO properly
      _tau_SUPG[qp] = (_doing_SUPG ? tauSUPG(qp) : 0);
      _tau_prime_SUPG[qp] = (_doing_SUPG ? tauPrimeSUPG(qp) : 0);
    }
}








RealVectorValue
RichardsMaterial::velSUPG(unsigned qp)
{
  // Bounds checking on element data and putting into vector form
  mooseAssert(qp < _dxidx.size(), "Insufficient data in dxidx array!");
  mooseAssert(qp < _dxidy.size(), "Insufficient data in dxidy array!");
  mooseAssert(qp < _dxidz.size(), "Insufficient data in dxidz array!");
  if (_mesh.dimension() >= 2)
    {
      mooseAssert(qp < _detadx.size(), "Insufficient data in detadx array!");
      mooseAssert(qp < _detady.size(), "Insufficient data in detady array!");
      mooseAssert(qp < _detadz.size(), "Insufficient data in detadz array!");
    }
  if (_mesh.dimension() >= 3)
    {
      mooseAssert(qp < _dzetadx.size(), "Insufficient data in dzetadx array!");
      mooseAssert(qp < _dzetady.size(), "Insufficient data in dzetady array!");
      mooseAssert(qp < _dzetadz.size(), "Insufficient data in dzetadz array!");
    }

  RealVectorValue a = -_material_perm*(_grad_p[qp] - _dens0[qp]*_gravity[qp]); // points in direction of info propagation

  return a;
}


RealTensorValue
RichardsMaterial::velPrimeSUPG(unsigned qp)
{
  return -_material_perm;
}


Real
RichardsMaterial::tauSUPG(unsigned qp)
{
  // CHECK : Does this work in 2D, 1D, spherical, cylindrical, etc?
  RealVectorValue xi_prime(_dxidx[qp], _dxidy[qp], _dxidz[qp]);
  RealVectorValue eta_prime, zeta_prime;
  if (_mesh.dimension() >= 2)
    {
      eta_prime(0) = _detadx[qp];
      eta_prime(1) = _detady[qp];
    }
  if (_mesh.dimension() == 3)
    {
      eta_prime(2) = _detadz[qp];
      zeta_prime(0) = _dzetadx[qp];
      zeta_prime(1) = _dzetady[qp];
      zeta_prime(2) = _dzetadz[qp];
    }

  RealVectorValue a = -_material_perm*(_grad_p[qp] - _dens0[qp]*_gravity[qp]); // points in direction of info propagation

  /* here i use a formula for "tau" presented in Appendix A of
     TJR Hughes, M Mallet and A Mizukami ``A new finite element formulation for computational fluid dynamics:: II. Behond SUPG'' Computer Methods in Applied Mechanics and Engineering 54 (1986) 341--355
  */
  Real norm_a = std::pow(a*a, 0.5);

  RealVectorValue b;
  b(0) = xi_prime*a;
  if (_mesh.dimension() >= 2)
    {
      b(1) = eta_prime*a;
    }
  if (_mesh.dimension() == 3)
    {
      b(2) = zeta_prime*a;
    }

  Real norm_b = std::pow(b*b, 0.5); // Hughes et al investigate infinity-norm and 2-norm.  i just use 2-norm here.   norm_b ~ 2|a|/ele_length_in_direction_of_a
  if (norm_b == 0)
    {
      return 0.0; // Only way for norm_b=0 is for zero ele size, or a=0.  Either way we don't have to upwind.
    }
  Real h = 2*norm_a/norm_b; // h is a measure of the element length in the "a" direction
  Real alpha = 0.5*norm_a*h/_trace_perm/_SUPG_pressure;

  // TEST TEST TEST
  //alpha *= 1E10; // TEST TEST TEST
  // TEST TEST TEST

  Real xi_tilde;
  if (alpha >= 20)
    {
      xi_tilde = 1 - 1.0/alpha; // prevents overflows
    }
  else if (alpha <= -20)
    {
      xi_tilde = -1 - 1.0/alpha;
    }
  else if (alpha == 0)
    {
      xi_tilde = 0.0;
    }
  else
    {
      xi_tilde = std::cosh(alpha)/std::sinh(alpha) - 1.0/alpha;
    }

  // TEST2
  //xi_tilde = 1.0;
    
  Real tau = (norm_b == 0 ? 0 : xi_tilde/norm_b);

  /*
  if (tau > 1E5){
    std::cout << "dim=" << _mesh.dimension() << "\n";
    std::cout << "xi_prime=" << xi_prime  << "\n";
    std::cout << "eta_prime=" << eta_prime  << "\n";
    std::cout << "zeta_prime=" << zeta_prime  << "\n";
    std::cout << "a=" << a << " b=" << b << "\n";
    std::cout << "h=" << h << " alpha=" << alpha << " xi_tilde=" <<  xi_tilde << " tau=" << tau << "\n";
  }
  */

  // Petrov-Galerkin test function is psi + tau*a.grad(psi)
  return tau;
}

RealVectorValue
RichardsMaterial::tauPrimeSUPG(unsigned qp)
{
  // CHECK : Does this work in 2D, 1D, spherical, cylindrical, etc?
  RealVectorValue xi_prime(_dxidx[qp], _dxidy[qp], _dxidz[qp]);
  RealVectorValue eta_prime, zeta_prime;
  if (_mesh.dimension() >= 2)
    {
      eta_prime(0) = _detadx[qp];
      eta_prime(1) = _detady[qp];
    }
  if (_mesh.dimension() == 3)
    {
      eta_prime(2) = _detadz[qp];
      zeta_prime(0) = _dzetadx[qp];
      zeta_prime(1) = _dzetady[qp];
      zeta_prime(2) = _dzetadz[qp];
    }

  /*
  In the following X_d_gradu is defined so that if X is in a kernel
  then X_d_gradu*_grad_phi[_j][_qp] is in the Jacobian.
  That is X_du = dX/d (_grad_u)
  */

  RealVectorValue a = -_material_perm*(_grad_p[qp] - _dens0[qp]*_gravity[qp]); // points in direction of info propagation
  RealTensorValue a_d_gradu = -_material_perm;

  Real norm_a = std::pow(a*a, 0.5);

  RealVectorValue norm_a_d_gradu(norm_a == 0 ? 0*a : a_d_gradu*a/norm_a); // ZERO properly

  RealVectorValue b;
  b(0) = xi_prime*a;
  if (_mesh.dimension() >= 2)
    {
      b(1) = eta_prime*a;
    }
  if (_mesh.dimension() == 3)
    {
      b(2) = zeta_prime*a;
    }

  Real norm_b = std::pow(b*b, 0.5); 
  RealVectorValue norm_b_d_gradu = ((xi_prime*a)*(a_d_gradu*xi_prime) + (eta_prime*a)*(a_d_gradu*eta_prime) + (zeta_prime*a)*(a_d_gradu*zeta_prime))/norm_b; // if a_d_gradu wasn't symmetric we'd have to do a transpose of it here.
  Real h(norm_b == 0 ? 0 : 2*norm_a/norm_b); // Only way for norm_b=0 is for zero ele size, or a=0.  Either way we don't have to upwind
  RealVectorValue h_d_gradu(norm_b == 0 ? 0*a : 2*norm_a_d_gradu/norm_b - 2*norm_a*norm_b_d_gradu/norm_b/norm_b); // ZERO properly
  Real alpha = 0.5*norm_a*h/_trace_perm/_SUPG_pressure;  // this is the Peclet number

  RealVectorValue alpha_d_gradu = 0.5*(norm_a_d_gradu*h + norm_a*h_d_gradu)/_trace_perm/_SUPG_pressure;

  // TEST TEST TEST
  //alpha *= 1E10; // TEST TEST TEST
  //alpha_d_gradu *= 1E10; // TEST TEST TEST
  // TEST TEST TEST

  Real xi_tilde;
  Real xi_tilde_prime; // d(xi_tilde)/d(alpha)
  if (alpha >= 20)
    {
      xi_tilde = 1 - 1.0/alpha; // prevents overflows
      xi_tilde_prime = 1.0/alpha/alpha;
    }
  else if (alpha <= -20)
    {
      xi_tilde = -1 - 1.0/alpha;
      xi_tilde_prime = 1.0/alpha/alpha;
    }
  else if (alpha == 0)
    {
      xi_tilde = 0.0;
      xi_tilde_prime = 1.0/3.0;
    }
  else
    {
      xi_tilde = std::cosh(alpha)/std::sinh(alpha) - 1.0/alpha;
      xi_tilde_prime = 1 - std::pow(std::cosh(alpha)/std::sinh(alpha), 2) + 1.0/alpha/alpha;
    }

  // TEST2
  //xi_tilde = 1.0;
  //xi_tilde_prime = 0.0;


  RealVectorValue xi_tilde_d_gradu = xi_tilde_prime*alpha_d_gradu;
  Real tau = (norm_b == 0 ? 0 : xi_tilde/norm_b);
  RealVectorValue tau_d_gradu = (norm_b == 0 ? 0*a : xi_tilde_d_gradu/norm_b - xi_tilde*norm_b_d_gradu/norm_b/norm_b); // ZERO properly

  return tau_d_gradu;

}
