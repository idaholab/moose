#include "SUPGMaterial.h"
#include "SUPG.h"
#include "FEProblem.h"       // for access to FEProblem::getNonlinearSystem()
#include "NonlinearSystem.h" // to get the current nonlinear iteration number

#include <limits> // std::numeric_limits<T>::epsilon()

// libMesh includes
#include "libmesh/quadrature.h"

registerMooseObject("THMApp", SUPGMaterial);

template <>
InputParameters
validParams<SUPGMaterial>()
{
  InputParameters params = validParams<Material>();

  // Conserved variables
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");

  params.addRequiredCoupledVar("A", "Cross-sectional");
  // Actual density, momentum, and total energy values.  Will be the same as above in the
  // constant-area equations, but not the variable-area equations.
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");

  // Coupled aux variables
  params.addRequiredCoupledVar("vel", "x-velocity");
  params.addRequiredCoupledVar("A", "cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("D_h", "hydraulic diameter");

  // Required parameters
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");

  // These parameters are only required if the energy equation is active
  params.addCoupledVar("H", "Specific total enthalpy");
  params.addCoupledVar("T", "fluid temperature aux variable");
  params.addCoupledVar("P_hf", "heat flux perimeter");
  params.addParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addParam<MaterialPropertyName>("Hw", "Wall heat transfer coefficient");

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factory material property");
  // optional parameters
  params.addParam<unsigned>(
      "n_iterations_before_freezing_delta", 2, "N. Newton iterations before freezing delta");

  // The EOS function is a required parameter.
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use");

  return params;
}

SUPGMaterial::SUPGMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    // The material properties
    _delta(declareProperty<Real>(SUPG::DELTA)),
    _A(declareProperty<RealTensorValue>(SUPG::MATRIX)),
    _R(declareProperty<RealVectorValue>(SUPG::RESIDUAL)),
    _y(declareProperty<std::vector<RealVectorValue>>(SUPG::COLUMNS)),
    _dUdx(declareProperty<RealVectorValue>(SUPG::DUDX)),
    _dA(declareProperty<std::vector<RealTensorValue>>(SUPG::DADU)),

    // Conserved variables
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _arhoEA(coupledValue("arhoEA")),

    _area(coupledValue("A")),
    // density, momentum, and total energy variables.  These will be the same as
    // the arhoA, arhouA, and arhoEA variables above for the constant-area
    // equations, but not the variable-area equations.
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),

    // Aux variables
    _enthalpy(coupledValue("H")),
    _temperature(coupledValue("T")),
    _has_heat_transfer(isParamValid("Hw") && isParamValid("T_wall")),
    _Hw(_has_heat_transfer ? getMaterialProperty<Real>("Hw") : getZeroMaterialProperty<Real>("Hw")),
    _vel(coupledValue("vel")),

    _p(getMaterialProperty<Real>("p")),
    _dp_drhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_drhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_drhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _D_h(getMaterialProperty<Real>("D_h")),
    _P_hf(coupledValue("P_hf")),

    // Time derivative values
    _ddt_arhoA(coupledDot("arhoA")),
    _ddt_arhouA(coupledDot("arhouA")),
    _ddt_arhoEA(coupledDot("arhoEA")),

    // Conserved variable gradients
    _grad_arhoA(coupledGradient("arhoA")),
    _grad_arhouA(coupledGradient("arhouA")),
    _grad_arhoEA(coupledGradient("arhoEA")),
    _grad_area(coupledGradient("A")),

    // Required parameters
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),
    _f_D(getMaterialProperty<Real>("f_D")),
    _T_wall(_has_heat_transfer ? getMaterialProperty<Real>("T_wall")
                               : getZeroMaterialProperty<Real>("T_wall")),

    // Optional parameters
    _n_iterations_before_freezing_delta(getParam<unsigned>("n_iterations_before_freezing_delta")),

    // If false during the call to computeProperties(), skips recomputing them,
    // i.e. uses the preceding values.
    _recompute_delta(true)
{
}

void
SUPGMaterial::computeProperties()
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  unsigned current_iteration_number =
      fe_problem->getNonlinearSystem().getCurrentNonlinearIterationNumber();

  // Libmesh now resets the current iteration number to 0 after the
  // nonlinear solve, so we should be able to use it to determine
  // whether to start freezing delta...
  if (current_iteration_number >= _n_iterations_before_freezing_delta)
    _recompute_delta = false;

  // Get a reference to the cached values for the current element.
  // (Create it if it does not yet exist.)
  std::vector<Real> & cached_delta_values = _cached_delta[_current_elem->id()];

  // Make sure there is enough space to store the cached values, one per quadrature point
  cached_delta_values.resize(_qrule->n_points());

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    Real rho = _rho[qp], rhou = _rhou[qp], rhoE = _rhoE[qp], u = _vel[qp], u2 = u * u,
         H = _enthalpy[qp], p = _p[qp];

    // Fill in vector of conserved variable derivatives for convenience
    _dUdx[qp].zero();
    _dUdx[qp](0) = _grad_arhoA[qp] * _dir[qp];
    _dUdx[qp](1) = _grad_arhouA[qp] * _dir[qp];
    _dUdx[qp](2) = _grad_arhoEA[qp] * _dir[qp];

    // The element size parameter.  In 1D this is simple.  In higher dimensions,
    // we need to do something more sophisticated...
    Real h = _current_elem->hmax();

    // Get derivatives of pressure wrt dependent vars from EOS object
    Real dp_drho = _dp_drhoA[qp] * _area[qp];
    Real dp_drhou = _dp_drhouA[qp] * _area[qp];
    Real dp_drhoE = _dp_drhoEA[qp] * _area[qp];

    // Fill in the advective flux matrix entries
    // Clear out any previous data, is this necessary?
    _A[qp].zero();

    // Fill in the entries of the advective flux matrix at this quadrature point.
    _A[qp](0, 0) = 0.;
    _A[qp](0, 1) = 1.;
    _A[qp](0, 2) = 0.;
    _A[qp](1, 0) = dp_drho - u2;
    _A[qp](1, 1) = dp_drhou + 2. * u;
    _A[qp](1, 2) = dp_drhoE;
    _A[qp](2, 0) = u * (dp_drho - H);
    _A[qp](2, 1) = u * dp_drhou + H;
    _A[qp](2, 2) = u * (1. + dp_drhoE);

    // Derivatives of flux Jacobian matrices wrt to the conserved variables.  These are useful
    // for computing Jacobians of SUPG terms.

    // Second derivatives of pressure are currently *neglected*.  This
    // is a good approximation for most equations of state I have
    // seen...
    Real p_00 = 0., p_01 = 0., p_02 = 0., p_10 = 0., p_11 = 0., p_12 = 0., p_20 = 0., p_21 = 0.,
         p_22 = 0.;

    // Make sure there is space in the vector...
    _dA[qp].resize(3);

    ///////////////
    // dA/d(rho)
    ///////////////
    _dA[qp][0].zero();

    // First row
    _dA[qp][0](0, 0) = 0.;
    _dA[qp][0](0, 1) = 0.;
    _dA[qp][0](0, 2) = 0.;

    // Second row
    _dA[qp][0](1, 0) = p_00 + 2. * u2 / rho;
    _dA[qp][0](1, 1) = p_10 - 2. * u / rho;
    _dA[qp][0](1, 2) = p_20;

    // Third row
    _dA[qp][0](2, 0) = u * (p_00 - 2. / rho * (dp_drho - H));
    _dA[qp][0](2, 1) = u * p_10 - (u / rho) * dp_drhou + (dp_drho - H) / rho;
    _dA[qp][0](2, 2) = u * p_20 - (u / rho) * (1. + dp_drhoE);

    ///////////////
    // dA/d(rhou)
    ///////////////
    _dA[qp][1].zero();

    // First row
    _dA[qp][1](0, 0) = 0.;
    _dA[qp][1](0, 1) = 0.;
    _dA[qp][1](0, 2) = 0.;

    // Second row
    _dA[qp][1](1, 0) = p_01 - 2. * u / rho;
    _dA[qp][1](1, 1) = p_11 - 2. / rho;
    _dA[qp][1](1, 2) = p_21;

    // Third row
    _dA[qp][1](2, 0) = u * (p_01 - dp_drhou / rho) - (u / rho) * (dp_drho - H);
    _dA[qp][1](2, 1) = u * p_11 + 2. * dp_drhou / rho;
    _dA[qp][1](2, 2) = (1. + dp_drhoE) / rho;

    ///////////////
    // dA/d(rhoE)
    ///////////////
    _dA[qp][2].zero();

    // First row
    _dA[qp][2](0, 0) = 0.;
    _dA[qp][2](0, 1) = 0.;
    _dA[qp][2](0, 2) = 0.;

    // Second row
    _dA[qp][2](1, 0) = p_02;
    _dA[qp][2](1, 1) = p_12;
    _dA[qp][2](1, 2) = p_22;

    // Third row
    _dA[qp][2](2, 0) = u * (p_02 - (1. + dp_drhoE) / rho);
    _dA[qp][2](2, 1) = u * p_12 + (1. + dp_drhoE) / rho;
    _dA[qp][2](2, 2) = u * p_22;

    // Miscellaneous terms comprising the strong-form residuals

    // Print suspiciously small velocity values.
    if (std::abs(u) > 0. && std::abs(u) < 1.e-99)
    {
      // FIXME: Figure out why this happens.  It seems to only happen for SMP_Newton.
      // Changing the -snes_ls type does not seem to affect the problem.  It usually goes
      // away after the first couple of timesteps in the water hammer problem, so it may
      // only be relevant for nearly zero velocity fields?

      // In the meantime: use a value that won't underflow when squared in the friction term...
      u = 0.;
    }

    // Note: These source terms are correct for both constant and variable-area equations
    Real gravity_term = -_arhoA[qp] * _dir[qp] * _gravity_vector;

    Real friction_term = 0.5 * _f_D[qp] * rho / _D_h[qp] * u * std::abs(u) * _area[qp];

    // Only attempt computing the heat source term if we have an
    // energy equation.
    Real heat_source_term =
        _has_heat_transfer ? _Hw[qp] * _P_hf[qp] * (_temperature[qp] - _T_wall[qp]) : 0;

    // p_hat is an additional source term that appears in the
    // quasi-linear form of the variable-area governing equations.
    // Since dA/dx=0 for the constant area equations, this term should
    // vanish in that case.
    Real p_hat = p - (dp_drho * rho + dp_drhou * rhou + dp_drhoE * rhoE);

    // Fill up the vector of strong residuals.
    _R[qp].zero();
    _R[qp](0) = _ddt_arhoA[qp];
    _R[qp](1) = _ddt_arhouA[qp] + gravity_term + friction_term - p * _grad_area[qp] * _dir[qp] +
                p_hat * _grad_area[qp] * _dir[qp];
    _R[qp](2) = _ddt_arhoEA[qp] + heat_source_term + u * gravity_term +
                u * p_hat * _grad_area[qp] * _dir[qp];

    // Complete the computation of the strong-form residuals with the convective flux term
    _R[qp] += (_A[qp] * _dUdx[qp]);

    // See compns notes for the origin of these terms... the rhs is
    // equivalent to computing the sound speed squared, but not all
    // EOS objects implement c2.
    Real must_be_positive = dp_drho + u * dp_drhou + H * dp_drhoE;

    if (must_be_positive < 0.)
      mooseError("EOS led to imaginary eigenvalues!");

    Real sqrt_term = std::sqrt(must_be_positive);

    Real lambda_1 = u, lambda_2 = u + sqrt_term, lambda_3 = u - sqrt_term,
         abs_lambda_1 = std::abs(lambda_1), abs_lambda_2 = std::abs(lambda_2),
         abs_lambda_3 = std::abs(lambda_3);

    // The d_j constants.  Again, see compns notes...
    Real d_2 = (H - u2) * (u * dp_drhoE - lambda_2) + u * (dp_drho - u2),
         d_3 = (H - u2) * (u * dp_drhoE - lambda_3) + u * (dp_drho - u2);

    // We have to divide by the d_j, so make sure they are non-zero
    if (!(std::abs(d_2) > std::numeric_limits<Real>::epsilon()) ||
        !(std::abs(d_3) > std::numeric_limits<Real>::epsilon()))
      mooseError("Can't divide by either d_2 or d_3 in SUPGMaterial!");

    // We also have to divide by dp/drho + lambda_1*dp/d(rhou), so make
    // sure that's not zero either...
    Real c_1_denom = (dp_drho + lambda_1 * dp_drhou);

    if (!(std::abs(c_1_denom) > std::numeric_limits<Real>::epsilon()))
      mooseError("Can't divide by c_1_denom in SUPGMaterial!");

    // And the c_j's
    Real c_1 = -dp_drhoE / c_1_denom, c_2 = -lambda_2 / d_2, c_3 = -lambda_3 / d_3;

    // detV is the determinant of the eigenvector matrix
    Real detV =
        c_1 * (c_2 - c_3) * lambda_1 + c_3 * (c_1 - c_2) * lambda_2 + c_2 * (c_3 - c_1) * lambda_3;

    // We need to divide by detV, so make sure it's not zero...
    Real detV_inverse = (std::abs(detV) > std::numeric_limits<Real>::epsilon() ? 1. / detV : 0.);

    if (detV_inverse == 0.)
      mooseError("detV == 0 in SUPGMaterial!");

    // Be careful when dividing by the lambdas.  In the 3-eqn case, this
    // is an actual danger, since u=0 is certainly possible to have!
    Real abs_lambda_1_inv =
             (abs_lambda_1 > std::numeric_limits<Real>::epsilon() ? 1. / abs_lambda_1 : 0.),
         abs_lambda_2_inv =
             (abs_lambda_2 > std::numeric_limits<Real>::epsilon() ? 1. / abs_lambda_2 : 0.),
         abs_lambda_3_inv =
             (abs_lambda_3 > std::numeric_limits<Real>::epsilon() ? 1. / abs_lambda_3 : 0.);

    // Create the (temporary) z_k vectors.
    std::vector<RealVectorValue> z(3);
    z[0] = RealVectorValue((lambda_2 * c_3 - lambda_3 * c_2) * abs_lambda_1_inv,
                           (lambda_3 * c_2 - lambda_1 * c_1) * abs_lambda_2_inv,
                           (lambda_1 * c_1 - lambda_2 * c_3) * abs_lambda_3_inv);

    z[1] = RealVectorValue((c_2 - c_3) * abs_lambda_1_inv,
                           (c_1 - c_2) * abs_lambda_2_inv,
                           (c_3 - c_1) * abs_lambda_3_inv);

    z[2] = RealVectorValue(c_2 * c_3 * (lambda_3 - lambda_2) * abs_lambda_1_inv,
                           c_1 * c_2 * (lambda_1 - lambda_3) * abs_lambda_2_inv,
                           c_1 * c_3 * (lambda_2 - lambda_1) * abs_lambda_3_inv);

    // Create the y_k vectors
    RealVectorValue c(c_1, c_3, c_2); // Note: 3 and 2 are reversed, this is correct
    RealVectorValue c_lambda(lambda_1 * c_1, lambda_2 * c_3, lambda_3 * c_2);
    RealVectorValue one(1., 1., 1.);

    _y[qp].resize(3);
    for (unsigned k = 0; k < _y[qp].size(); ++k)
    {
      // Compute individual entries of y[k]
      _y[qp][k](0) = c * z[k];
      _y[qp][k](1) = c_lambda * z[k];
      _y[qp][k](2) = one * z[k];

      // Scale y[k]
      _y[qp][k] *= (0.5 * h * detV_inverse);
    }

    // Compute the shock-capturing parameter, delta
    if (_recompute_delta)
    {
      const Real e = rhoE / rho - .5 * u2;

      // This value doesn't really make sense for liquids.  We need only
      // have gamma = 1 + epsilon, epsilon > 0 to guarantee positive-definiteness
      // of the shock-capturing norm.
      const Real gamma = 1. + .01;

      // Variable name shortcuts
      Real calE = 0.5 * u2;

      // Element parameters
      Real h2 = h * h, fourhm2 = 4. / h2;

      // A_0^{-1} matrix.  Note: we leave off the leading coefficient
      // of this matrix, (rho*e)^{-2}.  It cancels out
      // in the numerator and denominator anyway...
      Real A0m1[3][3] = {{rho * (calE * calE + gamma * e * e), -rho * u * calE, rho * (calE - e)},
                         {-rho * u * calE, rho * (e + u2), -rho * u},
                         {rho * (calE - e), -rho * u, rho}};

      // In 1D, grad(xi) . grad(U) = d(xi)/dx * [darhoA/dx, darhouA/dx]
      //                           = (2/h) * [darhoA/dx, darhouA/dx]

      // Compute ||R||^2 and ||grad(xi) . grad(U)||
      // (in A_0^{-1} norm)
      Real norm_R2 = 0., norm_xi2 = 0.;
      for (unsigned i = 0; i < 3; ++i)
        for (unsigned j = 0; j < 3; ++j)
        {
          norm_R2 += _R[qp](i) * A0m1[i][j] * _R[qp](j);
          norm_xi2 += fourhm2 * _dUdx[qp](i) * A0m1[i][j] * _dUdx[qp](j);
        }

      // If the denominator is zero, don't divide by it!
      _delta[qp] = std::sqrt(norm_R2 / (norm_xi2 + std::numeric_limits<float>::epsilon()));

      // Cache these values so that, if delta is frozen in the next Newton iteration, we can
      // utilize the cached value.
      cached_delta_values[qp] = _delta[qp];
    } // end if (_recompute_delta)
    else
    {
      // We are not recomputing mat. props, so set _delta[qp] from cached values
      _delta[qp] = cached_delta_values[qp];
    } // for recompute delta
  }   // for (qp)
}

void
SUPGMaterial::residualSetup()
{
}

void
SUPGMaterial::timestepSetup()
{
  // At the beginning of the timestep, turn computation of material properties back on
  // _console << "At beginning of timestep, turning _recompute_delta back on! " << std::endl;
  _recompute_delta = true;
}
