#include "NavierStokesMaterial.h"
#include "AssemblyData.h"

template<>
InputParameters validParams<NavierStokesMaterial>()
{
  InputParameters params = validParams<Material>();

  // Default is Air
  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");
  params.addRequiredParam<Real>("Pr", "Prandtl number.");
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // not required in 2D

  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredCoupledVar("enthalpy", "");
  
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "energy");

  return params;
}



NavierStokesMaterial::NavierStokesMaterial(const std::string & name,
                                           InputParameters parameters)
    :
    Material(name, parameters),
    _grad_u(coupledGradient("u")),
    _grad_v(coupledGradient("v")),
    _grad_w(_dim==3 ? coupledGradient("w") : _grad_zero),
   
    _viscous_stress_tensor(declareProperty<RealTensorValue>("viscous_stress_tensor")),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),

    // Declared here but _not_ calculated here
    // (See e.g. derived class, bighorn/include/materials/FluidTC1.h)
    _dynamic_viscosity(declareProperty<Real>("dynamic_viscosity")),
    
    // The momentum components of the inviscid flux Jacobians.
    _calA(declareProperty<std::vector<RealTensorValue> >("calA")),

    // "Velocity column" matrices
    _calC(declareProperty<std::vector<RealTensorValue> >("calC")),

    // Energy equation inviscid flux matrices, "cal E_{kl}" in the notes.
    _calE(declareProperty<std::vector<std::vector<RealTensorValue> > >("calE")),

    // Parameter values read in from input file
    _R(getParam<Real>("R")),
    _gamma(getParam<Real>("gamma")),
    _Pr(getParam<Real>("Pr")),
    _cv(getParam<Real>("cv")),
    
    // Coupled solution values needed for computing SUPG stabilization terms
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(_dim == 3 ? coupledValue("w") : _zero),

    _temperature(coupledValue("temperature")),
    _enthalpy(coupledValue("enthalpy")),

    // Coupled solution values
    _rho(coupledValue("rho")),
    _rho_u(coupledValue("rhou")),
    _rho_v(coupledValue("rhov")),
    _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
    _rho_e(coupledValue("rhoe")),
    
    // Time derivative values
    _drho_dt(coupledDot("rho")),
    _drhou_dt(coupledDot("rhou")),
    _drhov_dt(coupledDot("rhov")),
    _drhow_dt( _dim == 3 ? coupledDot("rhow") : _zero),
    _drhoe_dt(coupledDot("rhoe")),
    
    // Gradients
    _grad_rho(coupledGradient("rho")),
    _grad_rho_u(coupledGradient("rhou")),
    _grad_rho_v(coupledGradient("rhov")),
    _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),
    _grad_rho_e(coupledGradient("rhoe")),

    // Material properties for stabilization
    _hsupg(declareProperty<Real>("hsupg")),
    _tauc(declareProperty<Real>("tauc")),
    _taum(declareProperty<Real>("taum")),
    _taue(declareProperty<Real>("taue")),
    _strong_residuals(declareProperty<std::vector<Real> >("strong_residuals")),

    // Grab reference to linear Lagrange finite element object pointer,
    // currently this is always a linear Lagrange element, so this might need to
    // be generalized if we start working with higher-order elements...
    _fe(_subproblem.assembly(_tid).getFE(FEType())),
    
    // Grab references to FE object's mapping data
    _dxidx(_fe->get_dxidx()),
    _dxidy(_fe->get_dxidy()),
    _dxidz(_fe->get_dxidz()),
    _detadx(_fe->get_detadx()),
    _detady(_fe->get_detady()),
    _detadz(_fe->get_detadz()),
    // In 2D, these last three vectors will be empty...
    _dzetadx(_fe->get_dzetadx()),
    _dzetady(_fe->get_dzetady()),
    _dzetadz(_fe->get_dzetadz())
  {
    //Load these up in a vector for convenience
    _vel_grads.resize(3);

    _vel_grads[0] = &_grad_u;
    _vel_grads[1] = &_grad_v;
    _vel_grads[2] = &_grad_w;
  }


/**
 * Must be called _after_ the child class computes dynamic_viscosity.
 */
void
NavierStokesMaterial::computeProperties()
{  
  for (unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {  
    /******* Viscous Stress Tensor *******/
    //Technically... this _is_ the transpose (since we are loading these by rows)
    //But it doesn't matter....
    RealTensorValue grad_outter_u(_grad_u[qp],_grad_v[qp],_grad_w[qp]);

    grad_outter_u += grad_outter_u.transpose();

    Real div_vel = 0;
    for(unsigned int i=0;i<3;i++)
      div_vel += (*_vel_grads[i])[qp](i);

    //Add diagonal terms
    for(unsigned int i=0;i<3;i++)
      grad_outter_u(i,i) -= (2.0/3.0) * div_vel;

    grad_outter_u *= _dynamic_viscosity[qp];
    
    _viscous_stress_tensor[qp] = grad_outter_u;

    // Tabulated values of thermal conductivity vs. Temperature for air (k increases slightly with T):
    // T (K)    k (W/m-K)
    // 273      0.0243
    // 373      0.0314
    // 473      0.0386
    // 573      0.0454
    // 673      0.0515

    // Pr = (mu * cp) / k  ==>  k = (mu * cp) / Pr = (mu * gamma * cv) / Pr
    _thermal_conductivity[qp] = (_dynamic_viscosity[qp] * _gamma * _cv) / _Pr;

    // Compute stabilization parameters:

    // .) Compute SUPG element length scale.
    this->compute_h_supg(qp);
    //std::cout << "_hsupg[" << qp << "]=" << _hsupg[qp] << std::endl;

    // .) Compute SUPG parameter values.  (Must call this after compute_h_supg())
    this->compute_tau(qp);
    //std::cout << "_tauc[" << qp << "]=" << _tauc[qp] << ", ";
    //std::cout << "_taum[" << qp << "]=" << _taum[qp] << ", ";
    //std::cout << "_taue[" << qp << "]=" << _taue[qp] << std::endl;

    // .) Compute strong residual values.
    this->compute_strong_residuals(qp);
    // std::cout << "_strong_residuals[" << qp << "]=";
    // for (unsigned i=0; i<_strong_residuals[qp].size(); ++i)
    //   std::cout << _strong_residuals[qp][i] << " ";
    // std::cout << std::endl;
  }
}




void NavierStokesMaterial::compute_h_supg(unsigned qp)
{
  // Bounds checking on element data
  mooseAssert(qp < _dxidx.size(), "Insufficient data in dxidx array!");
  mooseAssert(qp < _dxidy.size(), "Insufficient data in dxidy array!");
  mooseAssert(qp < _dxidz.size(), "Insufficient data in dxidz array!");
    
  mooseAssert(qp < _detadx.size(), "Insufficient data in detadx array!");
  mooseAssert(qp < _detady.size(), "Insufficient data in detady array!");
  mooseAssert(qp < _detadz.size(), "Insufficient data in detadz array!");
    
  if (_dim == 3)
  {
    mooseAssert(qp < _dzetadx.size(), "Insufficient data in dzetadx array!");
    mooseAssert(qp < _dzetady.size(), "Insufficient data in dzetady array!");
    mooseAssert(qp < _dzetadz.size(), "Insufficient data in dzetadz array!");
  }

  // The velocity vector at this quadrature point.
  RealVectorValue U(_u_vel[qp],_v_vel[qp],_w_vel[qp]);

  // Pull out element inverse map values at the current qp into a little dense matrix
  Real dxi_dx[LIBMESH_DIM][LIBMESH_DIM];
  
  dxi_dx[0][0] = _dxidx[qp];  dxi_dx[0][1] = _dxidy[qp];
  dxi_dx[1][0] = _detadx[qp]; dxi_dx[1][1] = _detady[qp];

  // OK to access third entries on 2D elements if LIBMESH_DIM==3, though they
  // may be zero...
  if (LIBMESH_DIM == 3)
  {
    /**/             /**/               dxi_dx[0][2] = _dxidz[qp];
    /**/             /**/               dxi_dx[1][2] = _detadz[qp];
  }

  // The last row of entries available only for 3D elements.
  if (_dim == 3)
  {
    dxi_dx[2][0] = _dzetadx[qp];   dxi_dx[2][1] = _dzetady[qp];   dxi_dx[2][2] = _dzetadz[qp];
  }

  // Zero value before computing...
  _hsupg[qp]=0.;

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
      _hsupg[qp] += numer/denom;
  }


  // The constant 2 should be 1 if we are using triangular elements.
  _hsupg[qp] = 2. * sqrt(_hsupg[qp]);
}




void NavierStokesMaterial::compute_tau(unsigned qp)
{
  Real velmag = std::sqrt(_u_vel[qp]*_u_vel[qp] + 
			  _v_vel[qp]*_v_vel[qp] + 
			  _w_vel[qp]*_w_vel[qp]);

  // std::cout << "velmag=" << velmag << std::endl;

  // Make sure temperature >= 0 before trying to take sqrt
#ifdef DEBUG
  if (_temperature[qp] < 0.)
    mooseError("Negative temperature detected at quadrature point!");
#endif

  // The speed of sound for an ideal gas, sqrt(gamma * R * T)
  Real soundspeed = std::sqrt(_gamma * _R * _temperature[qp]);

  // If velmag == 0, then h should be zero as well. And we will return 0.
  if (velmag == 0.)
  {
    _tauc[qp] = 0.;
    _taum[qp] = 0.;
    _taue[qp] = 0.;
  }
  else
  {
    // The element length parameter, squared
    Real h2 = _hsupg[qp]*_hsupg[qp];

    // The viscosity-based term
    Real visc_term = _dynamic_viscosity[qp] / _rho[qp] / h2;
    
    // The thermal conductivity-based term, cp = gamma * cv
    Real k_term = _thermal_conductivity[qp] / _rho[qp] / (_gamma*_cv) / h2;

    // Standard compressible flow tau.  Does not account for low Mach number
    // limit.
    _tauc[qp] = _hsupg[qp] / (velmag + soundspeed);

    // Inspired by Hauke, the sum of the compressible and incompressible tauc.
    // This seems to give relatively large values when u is small, e.g.
    // h_supg=7.071068e-03, velmag=1.077553e-04 => tauc=6.562155e+01??
//    _tauc =
//      _hsupg[qp] / (velmag + soundspeed) + 
//      _hsupg[qp] / (velmag);

    // From Wong 2001.  This tau is O(M^2) for small M.  At small M,
    // tauc dominates the inverse square sums and basically makes
    // taum=taue=tac.  However, all my flows occur at low Mach numbers,
    // so there would basically never be any stabilization...
//    tauc = (_hsupg[qp] * velmag) / (velmag*velmag + soundspeed*soundspeed);

    // (tau_c)^{-2}
    Real taucm2 = 1./_tauc[qp]/_tauc[qp];

    _taum[qp] = 1. / std::sqrt(taucm2 + visc_term*visc_term);

    _taue[qp] = 1. / std::sqrt(taucm2 + k_term*k_term);
  }
  
}




void NavierStokesMaterial::compute_strong_residuals(unsigned qp)
{
  // Create storage at this qp for the strong residuals of all the equations.
  // In 2D, the value for the z-velocity equation will just be zero.
  _strong_residuals[qp].resize(5);
  
  // The timestep is stored in the Problem object, which can be accessed through
  // the parent pointer of the SubProblemInterface.  Don't need this if we are not
  // approximating time derivatives ourselves.
  // Real dt = _subproblem.parent()->dt();
  //std::cout << "dt=" << dt << std::endl;

  // Vector object for the velocity
  RealVectorValue vel(_u_vel[qp], _v_vel[qp], _w_vel[qp]);

  // A VectorValue object containing all zeros.  Makes it easier to
  // construct type tensor objects
  RealVectorValue zero(0., 0., 0.);

  // Velocity vector magnitude squared
  Real velmag2 = vel.size_sq();

  // Debugging: How large are the time derivative parts of the strong residuals?
//  std::cout << "drho_dt=" << _drho_dt
//	    << ", drhou_dt=" << _drhou_dt
//	    << ", drhov_dt=" << _drhov_dt
//	    << ", drhow_dt=" << _drhow_dt
//	    << ", drhoe_dt=" << _drhoe_dt
//	    << std::endl;

  // Momentum divergence
  Real divU = _grad_rho_u[qp](0) + _grad_rho_v[qp](1) + _grad_rho_w[qp](2);

  // Enough space to hold three space dimensions of velocity components at each qp,
  // regardless of what dimension we are actually running in.
  _calC[qp].resize(3);

  // Explicitly zero the calC
  for (unsigned i=0; i<3; ++i)
    _calC[qp][i].zero();

  // x-column matrix
  _calC[qp][0](0,0) = _u_vel[qp];
  _calC[qp][0](1,0) = _v_vel[qp];
  _calC[qp][0](2,0) = _w_vel[qp];

  // y-column matrix
  _calC[qp][1](0,1) = _u_vel[qp];
  _calC[qp][1](1,1) = _v_vel[qp];
  _calC[qp][1](2,1) = _w_vel[qp];

  // z-column matrix (this assumes LIBMESH_DIM==3!)
  _calC[qp][2](0,2) = _u_vel[qp];
  _calC[qp][2](1,2) = _v_vel[qp];
  _calC[qp][2](2,2) = _w_vel[qp];

  // The matrix S can be computed from any of the calC via calC_1*calC_1^T
  RealTensorValue calS = _calC[qp][0] * _calC[qp][0].transpose();

  // Enough space to hold five (=n_sd + 2) 3*3 calA matrices at this qp, regarless of dimension
  _calA[qp].resize(5);

  // 0.) _calA_0 = diag( (gam-1)/2*|u|^2 ) - S
  _calA[qp][0].zero(); // zero this calA entry
  _calA[qp][0](0,0) = _calA[qp][0](1,1) = _calA[qp][0](2,2) = 0.5*(_gamma-1.)*velmag2; // set diag. entries
  _calA[qp][0] -= calS;

  for (unsigned m=1; m<=3; ++m)
  {
    // Use m_local when indexing into matrices and vectors
    unsigned m_local = m-1;

    // For m=1,2,3, calA_m = C_m + C_m^T + diag( (1.-gam)*u_m )
    _calA[qp][m].zero(); // zero this calA entry
    _calA[qp][m](0,0) = _calA[qp][m](1,1) = _calA[qp][m](2,2) = (1.-_gamma)*vel(m_local); // set diag. entries
    _calA[qp][m] += _calC[qp][m_local];             // Note: use m_local for indexing into _calC!
    _calA[qp][m] += _calC[qp][m_local].transpose(); // Note: use m_local for indexing into _calC!
  }

  // 4.) calA_4 = diag(gam-1)
  _calA[qp][4].zero(); // zero this calA entry
  _calA[qp][4](0,0) = _calA[qp][4](1,1) = _calA[qp][4](2,2) = (_gamma-1.);

  // Enough space to hold the 3*5 "cal E" matrices which comprise the inviscid flux term
  // of the energy equation.  See notes for additional details
  _calE[qp].resize(3); // Three rows, 5 entries in each row

  for (unsigned k=0; k<3; ++k)
  {
    // Make enough room to store all 5 E matrices for this k
    _calE[qp][k].resize(5);

    // Store and reuse the velocity column transpose matrix for the
    // current value of k.
    RealTensorValue Ck_T = _calC[qp][k].transpose();

    // E_{k0} (density gradient term)
    _calE[qp][k][0].zero();
    _calE[qp][k][0] = (0.5*(_gamma-1)*velmag2 - _enthalpy[qp]) * Ck_T;

    for (unsigned m=1; m<=3; ++m)
    {
      // Use m_local when indexing into matrices and vectors
      unsigned m_local = m-1;

      // E_{km} (momentum gradient terms)
      _calE[qp][k][m].zero();
      _calE[qp][k][m](k,m_local) = _enthalpy[qp];           // H * D_{km}
      _calE[qp][k][m] += (1.-_gamma) * vel(m_local) * Ck_T; // (1-gam) * u_m * C_k^T
    }
    
    // E_{k4} (energy gradient term)
    _calE[qp][k][4].zero();
    _calE[qp][k][4] = _gamma * Ck_T;
  }
  
  // Compute the sum over ell of: A_ell grad(U_ell), store in DenseVector or Gradient object?
  // The gradient object might be more useful, since we are multiplying by VariableGradient 
  // (which is a MooseArray of RealGradients) objects?
  RealVectorValue mom_resid = 
    _calA[qp][0]*_grad_rho[qp] +
    _calA[qp][1]*_grad_rho_u[qp] +
    _calA[qp][2]*_grad_rho_v[qp] +
    _calA[qp][3]*_grad_rho_w[qp] +
    _calA[qp][4]*_grad_rho_e[qp];
  
  // No matrices/vectors for the energy residual strong form... just write it out like
  // the mass equation residual.  See "Momentum SUPG terms prop. to energy residual" 
  // section of the notes.
  Real energy_resid = 
    (0.5*(_gamma-1.)*velmag2 - _enthalpy[qp])*(vel * _grad_rho[qp]) +
    _enthalpy[qp]*divU +
    (1.-_gamma)*(vel(0)*(vel*_grad_rho_u[qp]) +
		 vel(1)*(vel*_grad_rho_v[qp]) +
		 vel(2)*(vel*_grad_rho_w[qp])) +
    _gamma*(vel*_grad_rho_e[qp])
    ;

  // Now for the actual residual values...

  // The density strong-residual
  _strong_residuals[qp][0] = _drho_dt[qp] + divU;

  // The x-momentum strong-residual, viscous terms neglected.
  // TODO: If we want to add viscous contributions back in, should this kernel
  // not inherit from NSViscousFluxBase so it can get tau values?  This would
  // also involve shape function second derivative values.
  _strong_residuals[qp][1] = _drhou_dt[qp] + mom_resid(0);

  // The y-momentum strong residual, viscous terms neglected.
  _strong_residuals[qp][2] = _drhov_dt[qp] + mom_resid(1);

  // The z-momentum strong residual, viscous terms neglected.
  if (_dim == 3)
    _strong_residuals[qp][3] = _drhow_dt[qp] + mom_resid(2);
  else
    _strong_residuals[qp][3] = 0.;

  // The energy equation strong residual
  _strong_residuals[qp][4] = _drhoe_dt[qp] + energy_resid;
}
