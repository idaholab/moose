//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Class outside the Moose hierarchy that contains common
 * functionality for computing derivatives of the viscous
 * stress tensor.
 *
 * This class is templated so that it can be used by either
 * a Kernel object or a BC object.
 */
template <class T>
class NSViscStressTensorDerivs
{
public:
  NSViscStressTensorDerivs(T & x);

  /**
   * The primary interface for computing viscous stress
   * tensor derivatives.  Requires access to protected data
   * from the the underlying data.  Uses index notation from
   * the notes.
   */
  Real dtau(unsigned k, unsigned ell, unsigned m);

private:
  T & _data;
};

template <class T>
NSViscStressTensorDerivs<T>::NSViscStressTensorDerivs(T & x) : _data(x)
{
}

template <class T>
Real
NSViscStressTensorDerivs<T>::dtau(unsigned k, unsigned ell, unsigned m)
{
  // Try to access underlying data.  Since this class is a friend, we can
  // directly access _qp and other protected data.  This only works if the
  // individual variables have the **same names** in all types T which may
  // be used to construct this class.

  //
  // Some error checking on input indices...
  //

  // 0 <= k,ell <= 2
  if (k > 2 || ell > 2)
    mooseError("Error, 0 <= k,ell <= 2 violated!");

  // 0 <= m <= 4
  if (m >= 5)
    mooseError("Error, m <= 4 violated!");

  //
  // Convenience variables
  //

  const Real rho = _data._rho[_data._qp];
  const Real rho2 = rho * rho;
  const Real phij = _data._phi[_data._j][_data._qp];

  const Real mu = _data._dynamic_viscosity[_data._qp];
  const Real nu = mu / rho;

  const RealVectorValue U(
      _data._rho_u[_data._qp], _data._rho_v[_data._qp], _data._rho_w[_data._qp]);

  const Real divU = _data._grad_rho_u[_data._qp](0) + _data._grad_rho_v[_data._qp](1) +
                    _data._grad_rho_w[_data._qp](2);

  // This makes a copy...but the resulting code is cleaner
  std::vector<RealVectorValue> gradU(3);
  gradU[0] = _data._grad_rho_u[_data._qp];
  gradU[1] = _data._grad_rho_v[_data._qp];
  gradU[2] = _data._grad_rho_w[_data._qp];

  // So we can refer to gradients without repeated indexing.
  const RealVectorValue & grad_phij = _data._grad_phi[_data._j][_data._qp];
  const RealVectorValue & grad_rho = _data._grad_rho[_data._qp];

  switch (m)
  {
    case 0: // density
    {
      const Real term1 = 2.0 / rho2 * (U(k) * grad_rho(ell) + U(ell) * grad_rho(k)) * phij;
      const Real term2 = -1.0 / rho *
                         ((gradU[k](ell) + gradU[ell](k)) * phij +
                          (U(k) * grad_phij(ell) + U(ell) * grad_phij(k)));

      // Kronecker delta terms
      Real term3 = 0.0;
      Real term4 = 0.0;
      if (k == ell)
      {
        term3 = -4.0 / 3.0 / rho2 * (U * grad_rho) * phij;
        term4 = 2.0 / 3.0 / rho * (U * grad_phij + divU * phij);
      }

      // Sum up result and return
      return nu * (term1 + term2 + term3 + term4);
    }

    // momentums
    case 1:
    case 2:
    case 3:
    {
      // note: when comparing m to k or ell, or indexing into Points,
      // must map m -> 0, 1, 2 by subtracting 1.
      const unsigned m_local = m - 1;

      // Kronecker delta terms
      const Real delta_km = (k == m_local ? 1.0 : 0.0);
      const Real delta_ellm = (ell == m_local ? 1.0 : 0.0);
      const Real delta_kell = (k == ell ? 1.0 : 0.0);

      return nu *
             (
                 /*     */ delta_km * (grad_phij(ell) - (phij / rho) * grad_rho(ell)) +
                 /*     */ delta_ellm * (grad_phij(k) - (phij / rho) * grad_rho(k)) -
                 (2. / 3.) * delta_kell * (grad_phij(m_local) - (phij / rho) * grad_rho(m_local)));
    } // end case 1,2,3

    case 4:
      // Derivative wrt to energy variable is zero.
      return 0.;

    default:
      mooseError("Invalid variable requested.");
      break;
  }

  return 0.;
}
