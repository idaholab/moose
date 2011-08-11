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
      
      // SUPG-related material properties
      _hsupg(getMaterialProperty<Real>("hsupg")),
      _tauc(getMaterialProperty<Real>("tauc")),
      _taum(getMaterialProperty<Real>("taum")),
      _taue(getMaterialProperty<Real>("taue")),
      _strong_residuals(getMaterialProperty<std::vector<Real> >("strong_residuals")),

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
      _cv(getParam<Real>("cv"))

      // These are now material properties
//      // tau values
//      _tauc(0.),
//      _taum(0.),
//      _taue(0.),
//
//      // Strong residuals vector.  Should be size _dim+2
//      _strong_residuals(_dim+2)
{
}



  // (material property) Real NSSUPGBase::h_supg()
  // (material property) {
  // (material property)   // The velocity vector at this quadrature point.
  // (material property)   RealVectorValue U(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);
  // (material property) 
  // (material property)   // From this kernel's MooseVariable object, grab a pointer to the underlying FE object.
  // (material property)   // (This requires an update to Moose to make this available.)
  // (material property)   FEBase * const & fe = this->_var.currentFE();
  // (material property) 
  // (material property)   // Get references of vectors of inverse Jacobian values at each quadrature point.
  // (material property)   const std::vector<Real>& dxidx = fe->get_dxidx();
  // (material property)   const std::vector<Real>& dxidy = fe->get_dxidy();
  // (material property)   const std::vector<Real>& dxidz = fe->get_dxidz();
  // (material property) 
  // (material property)   const std::vector<Real>& detadx = fe->get_detadx();
  // (material property)   const std::vector<Real>& detady = fe->get_detady();
  // (material property)   const std::vector<Real>& detadz = fe->get_detadz();
  // (material property) 
  // (material property)   // In 2D, these vectors will be empty
  // (material property)   const std::vector<Real>& dzetadx = fe->get_dzetadx();
  // (material property)   const std::vector<Real>& dzetady = fe->get_dzetady();
  // (material property)   const std::vector<Real>& dzetadz = fe->get_dzetadz();
  // (material property) 
  // (material property)   // Make sure that storage has at least been allocated for the inverse Jacobian data...
  // (material property)   mooseAssert(_qp < dxidx.size(), "Insufficient data in dxidx array!");
  // (material property)   mooseAssert(_qp < dxidy.size(), "Insufficient data in dxidy array!");
  // (material property)   mooseAssert(_qp < dxidz.size(), "Insufficient data in dxidz array!");
  // (material property)   
  // (material property)   mooseAssert(_qp < detadx.size(), "Insufficient data in detadx array!");
  // (material property)   mooseAssert(_qp < detady.size(), "Insufficient data in detady array!");
  // (material property)   mooseAssert(_qp < detadz.size(), "Insufficient data in detadz array!");
  // (material property)   
  // (material property)   // This storage will only exist on 3D elements...
  // (material property)   if (_dim == 3)
  // (material property)   {
  // (material property)     mooseAssert(_qp < dzetadx.size(), "Insufficient data in dzetadx array!");
  // (material property)     mooseAssert(_qp < dzetady.size(), "Insufficient data in dzetady array!");
  // (material property)     mooseAssert(_qp < dzetadz.size(), "Insufficient data in dzetadz array!");
  // (material property)   }
  // (material property)   
  // (material property) 
  // (material property)   // Pull out values at the current qp into a little dense matrix
  // (material property)   Real dxi_dx[LIBMESH_DIM][LIBMESH_DIM];
  // (material property)   
  // (material property)   dxi_dx[0][0] = dxidx[_qp];  dxi_dx[0][1] = dxidy[_qp];
  // (material property)   dxi_dx[1][0] = detadx[_qp]; dxi_dx[1][1] = detady[_qp];
  // (material property) 
  // (material property)   // OK to access third entries on 2D elements if LIBMESH_DIM==3, though they
  // (material property)   // may be zero...
  // (material property)   if (LIBMESH_DIM == 3)
  // (material property)   {
  // (material property)     /**/             /**/               dxi_dx[0][2] = dxidz[_qp];
  // (material property)     /**/             /**/               dxi_dx[1][2] = detadz[_qp];
  // (material property)   }
  // (material property) 
  // (material property)   // The last row of entries available only for 3D elements.
  // (material property)   if (_dim == 3)
  // (material property)   {
  // (material property)     dxi_dx[2][0] = dzetadx[_qp];   dxi_dx[2][1] = dzetady[_qp];   dxi_dx[2][2] = dzetadz[_qp];
  // (material property)   }
  // (material property) 
  // (material property)   // Value to be returned...
  // (material property)   Real h=0.;
  // (material property) 
  // (material property)   // Loop over k first, this is using the indexing as given in Bova and Ben's paper...
  // (material property)   // Only loop up to _dim not LIBMESH_DIM
  // (material property)   for (unsigned k=0; k<static_cast<unsigned>(_dim); ++k)
  // (material property)   {
  // (material property)     Real numer = U(k) * U(k);
  // (material property)     
  // (material property)     // Compute the denominator (fixed k)
  // (material property)     Real denom=0.;
  // (material property)     for (unsigned i=0; i<static_cast<unsigned>(_dim); ++i)
  // (material property)       for (unsigned j=0; j<static_cast<unsigned>(_dim); ++j)
  // (material property) 	denom += U(j) * dxi_dx[k][j] * U(i) * dxi_dx[k][i];
  // (material property) 
  // (material property)     // if the denom is identically zero, i.e. if U==0,
  // (material property)     // we obviously can't divide by it...  Note, we could check
  // (material property)     // for floating point equivalence to 0. instead of exact 0.0
  // (material property)     if (denom != 0.0)
  // (material property)       h += numer/denom;
  // (material property)   }
  // (material property) 
  // (material property) 
  // (material property)   // The constant 2 should be 1 if we are using triangular elements.
  // (material property)   return 2. * sqrt(h);
  // (material property) }






  // (material property) void NSSUPGBase::compute_tau()
  // (material property) {
  // (material property)   // The flow-aligned length scale value at this quadrature point
  // (material property)   Real h = this->h_supg();
  // (material property) 
  // (material property)   // std::cout << "h_supg=" << h << std::endl;
  // (material property) 
  // (material property)   Real velmag = std::sqrt(_u_vel[_qp]*_u_vel[_qp] + 
  // (material property) 			  _v_vel[_qp]*_v_vel[_qp] + 
  // (material property) 			  _w_vel[_qp]*_w_vel[_qp]);
  // (material property) 
  // (material property)   // std::cout << "velmag=" << velmag << std::endl;
  // (material property) 
  // (material property)   // The speed of sound for an ideal gas, sqrt(gamma * R * T)
  // (material property)   Real soundspeed = std::sqrt(_gamma * _R * _temperature[_qp]);
  // (material property)   
  // (material property)   // If velmag == 0, then h should be zero as well. And we will return 0.
  // (material property)   if (velmag == 0.)
  // (material property)   {
  // (material property)     this->_tauc = 0.;
  // (material property)     this->_taum = 0.;
  // (material property)     this->_taue = 0.;
  // (material property)   }
  // (material property)   else
  // (material property)   {
  // (material property)     // The viscosity-based term
  // (material property)     Real visc_term = _dynamic_viscosity[_qp] / _rho[_qp] / (h*h);
  // (material property)     
  // (material property)     // The thermal conductivity-based term, cp = gamma * cv
  // (material property)     Real k_term = _thermal_conductivity[_qp] / _rho[_qp] / (_gamma*_cv) / (h*h);
  // (material property) 
  // (material property)     // Standard compressible flow tau.  Does not account for low Mach number
  // (material property)     // limit.
  // (material property)     this->_tauc = h / (velmag + soundspeed);
  // (material property) 
  // (material property)     // Inspired by Hauke, the sum of the compressible and incompressible tauc.
  // (material property)     // This seems to give relatively large values when u is small, e.g.
  // (material property)     // h_supg=7.071068e-03, velmag=1.077553e-04 => tauc=6.562155e+01??
  // (material property) //    this->_tauc =
  // (material property) //      h / (velmag + soundspeed) + 
  // (material property) //      h / (velmag);
  // (material property) 
  // (material property)     // From Wong 2001.  This tau is O(M^2) for small M.  At small M,
  // (material property)     // tauc dominates the inverse square sums and basically makes
  // (material property)     // taum=taue=tac.  However, all my flows occur at low Mach numbers,
  // (material property)     // so there would basically never be any stabilization...
  // (material property) //    tauc = (h * velmag) / (velmag*velmag + soundspeed*soundspeed);
  // (material property) 
  // (material property)     this->_taum = 1. / std::sqrt(1./this->_tauc/this->_tauc + visc_term*visc_term);
  // (material property) 
  // (material property)     this->_taue = 1. / std::sqrt(1./this->_tauc/this->_tauc + k_term*k_term);
  // (material property)   }
  // (material property)   
  // (material property) }



  // (material property) void NSSUPGBase::compute_strong_residuals()
  // (material property) {
  // (material property)   // Convenience vars:
  // (material property)   
  // (material property)   // The timestep is stored in the Problem object
  // (material property)   Real dt = _problem.dt();
  // (material property) 
  // (material property)   // Vector object for the velocity
  // (material property)   RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  // (material property) 
  // (material property)   // A vector object containing all zeros.  Makes it easier to
  // (material property)   // construct type tensor objects
  // (material property)   RealVectorValue zero(0., 0., 0.);
  // (material property) 
  // (material property)   // Velocity vector magnitude squared
  // (material property)   Real velmag2 = vel.size_sq();
  // (material property) 
  // (material property)   // (Finite difference approximated) time derivative values
  // (material property)   Real drho_dt  = (_rho[_qp]   - _rho_old[_qp]  ) / dt;
  // (material property)   Real drhou_dt = (_rho_u[_qp] - _rho_u_old[_qp]) / dt;
  // (material property)   Real drhov_dt = (_rho_v[_qp] - _rho_v_old[_qp]) / dt;
  // (material property)   Real drhow_dt = (_rho_w[_qp] - _rho_w_old[_qp]) / dt;
  // (material property)   Real drhoe_dt = (_rho_e[_qp] - _rho_e_old[_qp]) / dt;
  // (material property) 
  // (material property)   // Momentum divergence
  // (material property)   Real divU = _grad_rho_u[_qp](0) + _grad_rho_v[_qp](1) + _grad_rho_w[_qp](2);
  // (material property) 
  // (material property)   // The set of _dim+2 3x3 matrices comprising the momentum equation
  // (material property)   // inviscid residual component.  Note: the code is general enough for 3D, the
  // (material property)   // z-components will all be zero for 2D.
  // (material property)   TensorValue<Real> calA_0, calA_1, calA_2, calA_3, calA_4;  
  // (material property) 
  // (material property)   // The "velocity row" matrices.  The constructor takes TypeVector objects
  // (material property)   // representing rows, so these are the "transpose" of the velocity columns...
  // (material property)   TensorValue<Real> 
  // (material property)     calC_1T(vel, zero, zero),
  // (material property)     calC_2T(zero, vel, zero),
  // (material property)     calC_3T(zero, zero, vel);
  // (material property) 
  // (material property)   // The "velocity column" matrices
  // (material property)   TensorValue<Real> 
  // (material property)     calC_1 = calC_1T.transpose(), 
  // (material property)     calC_2 = calC_2T.transpose(),
  // (material property)     calC_3 = calC_3T.transpose();
  // (material property)   
  // (material property)   
  // (material property)   // The matrix S can be computed from any of the calC via calC_1*calC_1^T
  // (material property)   TensorValue<Real> calS = calC_1 * calC_1T;
  // (material property) 
  // (material property)   // 0.) calA_0 = diag( (gam-1)/2*|u|^2 ) - S
  // (material property)   calA_0(0,0) = calA_0(1,1) = calA_0(2,2) = 0.5*(_gamma-1.)*velmag2; // set diag. entries
  // (material property)   calA_0 -= calS;
  // (material property) 
  // (material property)   // 1.) calA_1 = C_1 + C_1^T + diag( (1.-gam)*u_1 )
  // (material property)   calA_1(0,0) = calA_1(1,1) = calA_1(2,2) = (1.-_gamma)*vel(0); // set diag. entries
  // (material property)   calA_1 += calC_1;
  // (material property)   calA_1 += calC_1T;
  // (material property)   
  // (material property)   // 2.) calA_2 = C_2 + C_2^T + diag( (1.-gam)*u_2 )
  // (material property)   calA_2(0,0) = calA_2(1,1) = calA_2(2,2) = (1.-_gamma)*vel(1); // set diag. entries
  // (material property)   calA_2 += calC_2;
  // (material property)   calA_2 += calC_2T;
  // (material property) 
  // (material property)   // 3.) calA_3 = C_3 + C_3^T + diag( (1.-gam)*u_3 )
  // (material property)   calA_3(0,0) = calA_3(1,1) = calA_3(2,2) = (1.-_gamma)*vel(2); // set diag. entries
  // (material property)   calA_3 += calC_3;
  // (material property)   calA_3 += calC_3T;
  // (material property) 
  // (material property)   // 4.) calA_4 = diag(gam-1)
  // (material property)   calA_4(0,0) = calA_4(1,1) = calA_4(2,2) = (_gamma-1.);
  // (material property) 
  // (material property)   // Compute the sum over ell of: A_ell grad(U_ell), store in DenseVector or Gradient object?
  // (material property)   // The gradient object might be more useful, since we are multiplying by VariableGradient 
  // (material property)   // (which is a MooseArray of RealGradients) objects?
  // (material property)   RealVectorValue mom_sum = 
  // (material property)     calA_0*_grad_rho[_qp] +
  // (material property)     calA_1*_grad_rho_u[_qp] +
  // (material property)     calA_2*_grad_rho_v[_qp] +
  // (material property)     calA_3*_grad_rho_w[_qp] +
  // (material property)     calA_4*_grad_rho_e[_qp];
  // (material property) 
  // (material property)   // No matrices/vectors for the energy residual strong form... just write it out like
  // (material property)   // the mass equation residual.  See "Momentum SUPG terms prop. to energy residual" 
  // (material property)   // section of the notes.
  // (material property)   Real energy_resid = 
  // (material property)     (0.5*(_gamma-1.)*velmag2 - _enthalpy[_qp])*(vel * _grad_rho[_qp]) +
  // (material property)     _enthalpy[_qp]*divU +
  // (material property)     (1.-_gamma)*(vel(0)*(vel*_grad_rho_u[_qp]) +
  // (material property) 		 vel(1)*(vel*_grad_rho_v[_qp]) +
  // (material property) 		 vel(2)*(vel*_grad_rho_w[_qp])) +
  // (material property)     _gamma*(vel*_grad_rho_e[_qp])
  // (material property)     ;
  // (material property) 
  // (material property)   // ...
  // (material property) 
  // (material property)   // The density strong-residual
  // (material property)   _strong_residuals[0] = drho_dt + divU;
  // (material property) 
  // (material property)   // The x-momentum strong-residual, viscous terms neglected.
  // (material property)   // TODO: If we want to add viscous contributions back in, should this kernel
  // (material property)   // not inherit from NSViscousFluxBase so it can get tau values?  This would
  // (material property)   // also involve shape function second derivative values.
  // (material property)   _strong_residuals[1] = drhou_dt + mom_sum(0);
  // (material property) 
  // (material property)   // The y-momentum strong residual, viscous terms neglected.
  // (material property)   _strong_residuals[2] = drhov_dt + mom_sum(1);
  // (material property) 
  // (material property)   // The z-momentum strong residual, viscous terms neglected.
  // (material property)   if (_dim == 3)
  // (material property)     _strong_residuals[3] = drhow_dt + mom_sum(2);
  // (material property)   
  // (material property)   // The energy equation residual is always stored in index _dim+1
  // (material property)   // regardless of 2 or 3D.
  // (material property)   _strong_residuals[_dim+1] = drhoe_dt + energy_resid;
  // (material property) }




