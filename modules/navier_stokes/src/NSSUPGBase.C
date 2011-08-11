#include "NSSUPGBase.h"
 

template<>
InputParameters validParams<NSSUPGBase>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "energy");

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D

  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredCoupledVar("enthalpy", "");

  // Global parameters
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  return params;
}




NSSUPGBase::NSSUPGBase(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      // Material properties
      _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
      _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
      _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),

      // Coupled variable values
      _rho(coupledValue("rho")),
      _rho_u(coupledValue("rhou")),
      _rho_v(coupledValue("rhov")),
      _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
      _rho_e(coupledValue("rhoe")),

      // Old coupled variable values
      _rho_old(coupledValueOld("rho")),
      _rho_u_old(coupledValueOld("rhou")),
      _rho_v_old(coupledValueOld("rhov")),
      _rho_w_old( _dim == 3 ? coupledValueOld("rhow") : _zero),
      _rho_e_old(coupledValueOld("rhoe")),

      // Coupled aux variables
      _u_vel(coupledValue("u")),
      _v_vel(coupledValue("v")),
      _w_vel(_dim == 3 ? coupledValue("w") : _zero),

      _temperature(coupledValue("temperature")),
      _enthalpy(coupledValue("enthalpy")),

      // Gradients
      _grad_rho(coupledGradient("rho")),
      _grad_rho_u(coupledGradient("rhou")),
      _grad_rho_v(coupledGradient("rhov")),
      _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),
      _grad_rho_e(coupledGradient("rhoe")),

      // Variable numberings
      _rho_var_number( coupled("rho") ),
      _rhou_var_number( coupled("rhou") ),
      _rhov_var_number( coupled("rhov") ),
      _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
      _rhoe_var_number( coupled("rhoe") ),

      // Global parameters
      _gamma(getParam<Real>("gamma")),
      _R(getParam<Real>("R")),
      _cv(getParam<Real>("cv")),

      // tau values
      _tauc(0.),
      _taum(0.),
      _taue(0.),

      // Strong residuals vector.  Should be size _dim+2
      _strong_residuals(_dim+2)
{
}



Real NSSUPGBase::h_supg()
{
  // The velocity vector at this quadrature point.
  RealVectorValue U(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  // From this kernel's MooseVariable object, grab a pointer to the underlying FE object.
  // (This requires an update to Moose to make this available.)
  FEBase * const & fe = this->_var.currentFE();

  // Get references of vectors of inverse Jacobian values at each quadrature point.
  const std::vector<Real>& dxidx = fe->get_dxidx();
  const std::vector<Real>& dxidy = fe->get_dxidy();
  const std::vector<Real>& dxidz = fe->get_dxidz();

  const std::vector<Real>& detadx = fe->get_detadx();
  const std::vector<Real>& detady = fe->get_detady();
  const std::vector<Real>& detadz = fe->get_detadz();

  // In 2D, these vectors will be empty
  const std::vector<Real>& dzetadx = fe->get_dzetadx();
  const std::vector<Real>& dzetady = fe->get_dzetady();
  const std::vector<Real>& dzetadz = fe->get_dzetadz();

  // Make sure that storage has at least been allocated for the inverse Jacobian data...
  mooseAssert(_qp < dxidx.size(), "Insufficient data in dxidx array!");
  mooseAssert(_qp < dxidy.size(), "Insufficient data in dxidy array!");
  mooseAssert(_qp < dxidz.size(), "Insufficient data in dxidz array!");
  
  mooseAssert(_qp < detadx.size(), "Insufficient data in detadx array!");
  mooseAssert(_qp < detady.size(), "Insufficient data in detady array!");
  mooseAssert(_qp < detadz.size(), "Insufficient data in detadz array!");
  
  // This storage will only exist on 3D elements...
  if (_dim == 3)
  {
    mooseAssert(_qp < dzetadx.size(), "Insufficient data in dzetadx array!");
    mooseAssert(_qp < dzetady.size(), "Insufficient data in dzetady array!");
    mooseAssert(_qp < dzetadz.size(), "Insufficient data in dzetadz array!");
  }
  

  // Pull out values at the current qp into a little dense matrix
  Real dxi_dx[LIBMESH_DIM][LIBMESH_DIM];
  
  dxi_dx[0][0] = dxidx[_qp];  dxi_dx[0][1] = dxidy[_qp];
  dxi_dx[1][0] = detadx[_qp]; dxi_dx[1][1] = detady[_qp];

  // OK to access third entries on 2D elements if LIBMESH_DIM==3, though they
  // may be zero...
  if (LIBMESH_DIM == 3)
  {
    /**/             /**/               dxi_dx[0][2] = dxidz[_qp];
    /**/             /**/               dxi_dx[1][2] = detadz[_qp];
  }

  // The last row of entries available only for 3D elements.
  if (_dim == 3)
  {
    dxi_dx[2][0] = dzetadx[_qp];   dxi_dx[2][1] = dzetady[_qp];   dxi_dx[2][2] = dzetadz[_qp];
  }

  // Value to be returned...
  Real h=0.;

  // Loop over k first, this is using the indexing as given in Bova and Ben's paper...
  // Only loop up to _dim not LIBMESH_DIM
  for (unsigned k=0; k<static_cast<unsigned>(_dim); ++k)
  {
    Real numer = U(k) * U(k);
    
    // Compute the denominator (fixed k)
    Real denom=0.;
    for (unsigned i=0; i<static_cast<unsigned>(_dim); ++i)
      for (unsigned j=0; j<static_cast<unsigned>(_dim); ++j)
	denom += U(j) * dxi_dx[k][j] * U(i) * dxi_dx[k][i];

    // if the denom is identically zero, i.e. if U==0,
    // we obviously can't divide by it...  Note, we could check
    // for floating point equivalence to 0. instead of exact 0.0
    if (denom != 0.0)
      h += numer/denom;
  }


  // The constant 2 should be 1 if we are using triangular elements.
  return 2. * sqrt(h);
}






void NSSUPGBase::compute_tau()
{
  // The flow-aligned length scale value at this quadrature point
  Real h = this->h_supg();

  // std::cout << "h_supg=" << h << std::endl;

  Real velmag = std::sqrt(_u_vel[_qp]*_u_vel[_qp] + 
			  _v_vel[_qp]*_v_vel[_qp] + 
			  _w_vel[_qp]*_w_vel[_qp]);

  // std::cout << "velmag=" << velmag << std::endl;

  // The speed of sound for an ideal gas, sqrt(gamma * R * T)
  Real soundspeed = std::sqrt(_gamma * _R * _temperature[_qp]);
  
  // If velmag == 0, then h should be zero as well. And we will return 0.
  if (velmag == 0.)
  {
    this->_tauc = 0.;
    this->_taum = 0.;
    this->_taue = 0.;
  }
  else
  {
    // The viscosity-based term
    Real visc_term = _dynamic_viscosity[_qp] / _rho[_qp] / (h*h);
    
    // The thermal conductivity-based term, cp = gamma * cv
    Real k_term = _thermal_conductivity[_qp] / _rho[_qp] / (_gamma*_cv) / (h*h);

    // Standard compressible flow tau.  Does not account for low Mach number
    // limit.
    this->_tauc = h / (velmag + soundspeed);

    // Inspired by Hauke, the sum of the compressible and incompressible tauc.
    // This seems to give relatively large values when u is small, e.g.
    // h_supg=7.071068e-03, velmag=1.077553e-04 => tauc=6.562155e+01??
//    this->_tauc =
//      h / (velmag + soundspeed) + 
//      h / (velmag);

    // From Wong 2001.  This tau is O(M^2) for small M.  At small M,
    // tauc dominates the inverse square sums and basically makes
    // taum=taue=tac.  However, all my flows occur at low Mach numbers,
    // so there would basically never be any stabilization...
//    tauc = (h * velmag) / (velmag*velmag + soundspeed*soundspeed);

    this->_taum = 1. / std::sqrt(1./this->_tauc/this->_tauc + visc_term*visc_term);

    this->_taue = 1. / std::sqrt(1./this->_tauc/this->_tauc + k_term*k_term);
  }
  
}



void NSSUPGBase::compute_strong_residuals()
{
  // Convenience vars:
  
  // The timestep is stored in the Problem object
  Real dt = _problem.dt();

  // Vector object for the velocity
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // A vector object containing all zeros.  Makes it easier to
  // construct type tensor objects
  RealVectorValue zero(0., 0., 0.);

  // Velocity vector magnitude squared
  Real velmag2 = vel.size_sq();

  // (Finite difference approximated) time derivative values
  Real drho_dt  = (_rho[_qp]   - _rho_old[_qp]  ) / dt;
  Real drhou_dt = (_rho_u[_qp] - _rho_u_old[_qp]) / dt;
  Real drhov_dt = (_rho_v[_qp] - _rho_v_old[_qp]) / dt;
  Real drhow_dt = (_rho_w[_qp] - _rho_w_old[_qp]) / dt;
  Real drhoe_dt = (_rho_e[_qp] - _rho_e_old[_qp]) / dt;

  // Momentum divergence
  Real divU = _grad_rho_u[_qp](0) + _grad_rho_v[_qp](1) + _grad_rho_w[_qp](2);

  // The set of _dim+2 3x3 matrices comprising the momentum equation
  // inviscid residual component.  Note: the code is general enough for 3D, the
  // z-components will all be zero for 2D.
  TensorValue<Real> calA_0, calA_1, calA_2, calA_3, calA_4;  

  // The "velocity row" matrices.  The constructor takes TypeVector objects
  // representing rows, so these are the "transpose" of the velocity columns...
  TensorValue<Real> 
    calC_1T(vel, zero, zero),
    calC_2T(zero, vel, zero),
    calC_3T(zero, zero, vel);

  // The "velocity column" matrices
  TensorValue<Real> 
    calC_1 = calC_1T.transpose(), 
    calC_2 = calC_2T.transpose(),
    calC_3 = calC_3T.transpose();
  
  
  // The matrix S can be computed from any of the calC via calC_1*calC_1^T
  TensorValue<Real> calS = calC_1 * calC_1T;

  // 0.) calA_0 = diag( (gam-1)/2*|u|^2 ) - S
  calA_0(0,0) = calA_0(1,1) = calA_0(2,2) = 0.5*(_gamma-1.)*velmag2; // set diag. entries
  calA_0 -= calS;

  // 1.) calA_1 = C_1 + C_1^T + diag( (1.-gam)*u_1 )
  calA_1(0,0) = calA_1(1,1) = calA_1(2,2) = (1.-_gamma)*vel(0); // set diag. entries
  calA_1 += calC_1;
  calA_1 += calC_1T;
  
  // 2.) calA_2 = C_2 + C_2^T + diag( (1.-gam)*u_2 )
  calA_2(0,0) = calA_2(1,1) = calA_2(2,2) = (1.-_gamma)*vel(1); // set diag. entries
  calA_2 += calC_2;
  calA_2 += calC_2T;

  // 3.) calA_3 = C_3 + C_3^T + diag( (1.-gam)*u_3 )
  calA_3(0,0) = calA_3(1,1) = calA_3(2,2) = (1.-_gamma)*vel(2); // set diag. entries
  calA_3 += calC_3;
  calA_3 += calC_3T;

  // 4.) calA_4 = diag(gam-1)
  calA_4(0,0) = calA_4(1,1) = calA_4(2,2) = (_gamma-1.);

  // Compute the sum over ell of: A_ell grad(U_ell), store in DenseVector or Gradient object?
  // The gradient object might be more useful, since we are multiplying by VariableGradient 
  // (which is a MooseArray of RealGradients) objects?
  RealVectorValue mom_sum = 
    calA_0*_grad_rho[_qp] +
    calA_1*_grad_rho_u[_qp] +
    calA_2*_grad_rho_v[_qp] +
    calA_3*_grad_rho_w[_qp] +
    calA_4*_grad_rho_e[_qp];

  // No matrices/vectors for the energy residual strong form... just write it out like
  // the mass equation residual.  See "Momentum SUPG terms prop. to energy residual" 
  // section of the notes.
  Real energy_resid = 
    (0.5*(_gamma-1.)*velmag2 - _enthalpy[_qp])*(vel * _grad_rho[_qp]) +
    _enthalpy[_qp]*divU +
    (1.-_gamma)*(vel(0)*(vel*_grad_rho_u[_qp]) +
		 vel(1)*(vel*_grad_rho_v[_qp]) +
		 vel(2)*(vel*_grad_rho_w[_qp])) +
    _gamma*(vel*_grad_rho_e[_qp])
    ;

  // ...

  // The density strong-residual
  _strong_residuals[0] = drho_dt + divU;

  // The x-momentum strong-residual, viscous terms neglected.
  // TODO: If we want to add viscous contributions back in, should this kernel
  // not inherit from NSViscousFluxBase so it can get tau values?  This would
  // also involve shape function second derivative values.
  _strong_residuals[1] = drhou_dt + mom_sum(0);

  // The y-momentum strong residual, viscous terms neglected.
  _strong_residuals[2] = drhov_dt + mom_sum(1);

  // The z-momentum strong residual, viscous terms neglected.
  if (_dim == 3)
    _strong_residuals[3] = drhow_dt + mom_sum(2);
  
  // The energy equation residual is always stored in index _dim+1
  // regardless of 2 or 3D.
  _strong_residuals[_dim+1] = drhoe_dt + energy_resid;
}




