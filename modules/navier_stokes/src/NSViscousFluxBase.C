#include "NSViscousFluxBase.h"
 

template<>
InputParameters validParams<NSViscousFluxBase>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D

  return params;
}




NSViscousFluxBase::NSViscousFluxBase(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      // Material properties
      _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
      _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),

      // Coupled variable values
      _rho(coupledValue("rho")),
      _rho_u(coupledValue("rhou")),
      _rho_v(coupledValue("rhov")),
      _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),

      // Gradients
      _grad_rho(coupledGradient("rho")),
      _grad_rho_u(coupledGradient("rhou")),
      _grad_rho_v(coupledGradient("rhov")),
      _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),

      // Variable numberings
      _rho_var_number( coupled("rho") ),
      _rhou_var_number( coupled("rhou") ),
      _rhov_var_number( coupled("rhov") ),
      _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint)
{
}




Real NSViscousFluxBase::dtau(unsigned k, unsigned ell, unsigned m)
{
  // 0 <= k,ell <= 2
  if ( (k>2) || (ell>2) )
    mooseError("Error, 0 <= k,ell <= 2 violated!");

  // 0 <= m <= 4
  if (m >= 5)
    mooseError("Error, m <= 4 violated!");

  // Convenience variables
  Real rho  = _rho[_qp];
  Real rho2 = rho*rho;
  Real phij = _phi[_j][_qp];

  Real mu = _dynamic_viscosity[_qp];
  Real nu = mu / rho;
  
  RealVectorValue U(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  Real divU = _grad_rho_u[_qp](0) + _grad_rho_v[_qp](1) + _grad_rho_w[_qp](2);

  // This makes a copy...but the resulting code is cleaner
  std::vector<RealVectorValue> gradU(3);
  gradU[0] = _grad_rho_u[_qp];
  gradU[1] = _grad_rho_v[_qp];
  gradU[2] = _grad_rho_w[_qp];

  // Fixed-size array of constant references?  You just can't do that...
  //const RealVectorValue& gradU[3] = { _grad_rho_u[_qp], _grad_rho_v[_qp], _grad_rho_w[_qp] };
  
  // This doesn't make a copy, but you have to dereference it in code below...
  // std::vector<RealVectorValue*> gradU(3);
  // gradU[0] = &_grad_rho_u[_qp];
  // gradU[1] = &_grad_rho_v[_qp];
  // gradU[2] = &_grad_rho_w[_qp];
  

  // So we can refer to gradients without passing additional indices.
  const RealVectorValue& grad_phij = _grad_phi[_j][_qp];
  const RealVectorValue& grad_rho  = _grad_rho[_qp];
  
  // ...

  switch ( m )
  {
  case 0: // density
  {
    Real term1 =  2./rho2 * (U(k)*grad_rho(ell) + U(ell)*grad_rho(k)) * phij;
    Real term2 = -1./rho*( (gradU[k](ell) + gradU[ell](k))*phij + (U(k)*grad_phij(ell) + U(ell)*grad_phij(k)) );

    // Kronecker delta terms
    Real term3 = 0.;
    Real term4 = 0.;
    if (k==ell)
    {
      term3 = -4./3./rho2 * (U*grad_rho) * phij;
      term4 = 2./3./rho * (U*grad_phij + divU*phij);
    }
    
    //std::cout << term1 << ", " << term2 << ", " << term3 << ", " << term4 << std::endl;

    // Sum up result and return
    return nu*(term1 + term2 + term3 + term4);
  }

  case 1: // momentums
  case 2:
  case 3:
  {
    // note: when comparing m to k or ell, or indexing into Points, 
    // must map m -> 0, 1, 2 by subtracting 1 or use m_local!
    const unsigned m_local = m-1;

    if (m_local == k) // on-diagonal, note: when comparing m to k or ell, must map m -> 0, 1, 2 by subtracting 1
    {
      Real term1 = grad_phij(ell) - (phij/rho)*grad_rho(ell);
      
      // Kronecker delta term
      Real term2 = 0;
      if (k==ell)
	term2 = (1./3.)*term1;

      // std::cout << term1 << ", " << term2 << std::endl;

      // Sum up and return
      return nu*(term1 + term2);
    }

    else // off-diagonal
    {
      // Kronecker delta terms
      Real term1 = 0.;
      Real term2 = 0.;

      if (m_local == ell) // note: when comparing m to k or ell, must map m -> 0, 1, 2 by subtracting 1
	term1 = grad_phij(k) - (phij/rho)*grad_rho(k);

      if (k == ell)
	term2 = (-2./3.)*(grad_phij(m_local) - (phij/rho)*grad_rho(m_local)); // must map m -> 0, 1, or 2 by subtracting 1

      // std::cout << term1 << ", " << term2 << std::endl;

      // Sum up and return
      return nu*(term1 + term2);
    }
  }

  case 4:
  {
    // Derivative wrt to energy variable is zero.
    return 0.;
  }

  default:
  {
    mooseError("Invalid variable requested.");
  }

  } // end switch(m)

  return 0.;
}
