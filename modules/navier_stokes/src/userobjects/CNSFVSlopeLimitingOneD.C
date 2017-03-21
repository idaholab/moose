/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVSlopeLimitingOneD.h"

template <>
InputParameters
validParams<CNSFVSlopeLimitingOneD>()
{
  InputParameters params = validParams<SlopeLimitingBase>();

  params.addClassDescription("A use object that serves as base class for slope limiting to get the "
                             "limited slopes of cell average variables in 1-D.");

  params.addRequiredCoupledVar("rho", "Density at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhou", "X-momentum at P0 (constant monomial)");

  params.addRequiredCoupledVar("rhoe", "Total energy at P0 (constant monomial)");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  MooseEnum scheme("none minmod mc superbee", "none");

  params.addParam<MooseEnum>("scheme", scheme, "Slope limiting scheme");

  return params;
}

CNSFVSlopeLimitingOneD::CNSFVSlopeLimitingOneD(const InputParameters & parameters)
  : SlopeLimitingBase(parameters),
    _rho(getVar("rho", 0)),
    _rhou(getVar("rhou", 0)),
    _rhoe(getVar("rhoe", 0)),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _scheme(getParam<MooseEnum>("scheme"))
{
}

std::vector<RealGradient>
CNSFVSlopeLimitingOneD::limitElementSlope() const
{
  const Elem * elem = _current_elem;

  /// index of conserved variable
  unsigned int iv;

  /// index of sides around an element
  unsigned int is;

  /// index of the neighbor element
  unsigned int in;

  /// number of sides surrounding an element = 2 in 1D
  unsigned int nside = elem->n_sides();

  /// number of reconstruction stencils = 3 in 1D
  unsigned int nsten = nside + 1;

  /// number of conserved variables = 3 in 1D
  unsigned int nvars = 5;

  /// vector for the gradients of primitive variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0.0, 0.0, 0.0));

  /// array to store center coordinates of this cell and its neighbor cells
  std::vector<Real> xc(nsten, 0.);

  /// the first always stores the current cell
  xc[0] = elem->centroid()(0);

  /// array for the cell-average values in the current cell and its neighbors
  std::vector<std::vector<Real>> ucell(nsten, std::vector<Real>(nvars, 0.0));

  /// array to store the central and one-sided slopes
  /// where the first should be central slope

  /// central slope:
  ///                  u_{i+1} - u {i-1}
  ///                  -----------------
  ///                  x_{i+1} - x_{i-1}

  /// left-side slope:
  ///                  u_{i} - u {i-1}
  ///                  ---------------
  ///                  x_{i} - x_{i-1}

  /// right-side slope:
  ///                  u_{i+1} - u {i}
  ///                  ---------------
  ///                  x_{i+1} - x_{i}

  std::vector<std::vector<Real>> sigma(nsten, std::vector<Real>(nvars, 0.0));

  /// get the cell-average conserved variables in the central cell
  /// iv = 0: rho
  /// iv = 1: rhou
  /// iv = 2: rhoe

  Real rho = _rho->getElementalValue(elem);
  Real rhou = _rhou->getElementalValue(elem);
  Real rhoe = _rhoe->getElementalValue(elem);

  /// convert into primitive variables
  /// iv = 0: pres
  /// iv = 1: uadv
  /// iv = 2: temp

  ucell[0][1] = rhou / rho;

  Real v = 1. / rho;
  Real e = rhoe / rho - 0.5 * ucell[0][1] * ucell[0][1];

  ucell[0][0] = _fp.pressure(v, e);
  ucell[0][4] = _fp.temperature(v, e);

  /// a flag to indicate the boundary side of the current cell

  unsigned int bflag = 0;

  /// loop over the sides to compute the one-sided slopes

  for (is = 0; is < nside; is++)
  {
    in = is + 1;

    /// when the current element is an internal cell
    if (elem->neighbor(is) != NULL)
    {
      const Elem * neig = elem->neighbor(is);

      xc[in] = neig->centroid()(0);

      /// get the cell-average conserved variables in this neighbor cell
      /// iv = 0: rho
      /// iv = 1: rhou
      /// iv = 2: rhoe

      rho = _rho->getElementalValue(neig);
      rhou = _rhou->getElementalValue(neig);
      rhoe = _rhoe->getElementalValue(neig);

      ///convert into primitive variables
      /// iv = 0: pres
      /// iv = 1: uadv
      /// iv = 2: temp

      ucell[in][1] = rhou / rho;

      v = 1. / rho;
      e = rhoe / rho - 0.5 * ucell[in][1] * ucell[in][1];

      ucell[in][0] = _fp.pressure(v, e);
      ucell[in][4] = _fp.temperature(v, e);

      /// calculate the one-sided slopes of primitive variables

      for (iv = 0; iv < nvars; iv++)
        sigma[in][iv] = (ucell[0][iv] - ucell[in][iv]) / (xc[0] - xc[in]);
    }
    /// When the current element is a boundary cell,
    /// in general we do not construct the slope for 1D,
    /// and the solution accuracy will not be impaired globally.
    /// But we should definitely do it for multi-D.
    /// Otherwise the accuracy of system reduces to first-order.
    else
    {
      bflag = in;
    }
  }

  /// calculate the slope of the current element
  /// according to the choice of the slope limiter algorithm

  switch (_scheme)
  {
    /// first-order, zero slope
    case 0:
      break;

    /// minmod limiter
    case 1:

      if (bflag == 0)
      {
        for (iv = 0; iv < nvars; iv++)
        {
          if ((sigma[1][iv] * sigma[2][iv]) > 0.)
          {
            if (std::abs(sigma[1][iv]) < std::abs(sigma[2][iv]))
              ugrad[iv](0) = sigma[1][iv];
            else
              ugrad[iv](0) = sigma[2][iv];
          }
        }
      }
      break;

    /// MC (monotonized central-difference limiter)
    case 2:

      if (bflag == 0)
      {
        for (iv = 0; iv < nvars; iv++)
          sigma[0][iv] = (ucell[1][iv] - ucell[2][iv]) / (xc[1] - xc[2]);

        for (iv = 0; iv < nvars; iv++)
        {
          if (sigma[0][iv] > 0. && sigma[1][iv] > 0. && sigma[2][iv] > 0.)
            ugrad[iv](0) = std::min(sigma[0][iv], 2. * std::min(sigma[1][iv], sigma[2][iv]));
          else if (sigma[0][iv] < 0. && sigma[1][iv] < 0. && sigma[2][iv] < 0.)
            ugrad[iv](0) = std::max(sigma[0][iv], 2. * std::max(sigma[1][iv], sigma[2][iv]));
        }
      }
      break;

    /// Superbee limiter
    case 3:

      if (bflag == 0)
      {
        for (iv = 0; iv < nvars; iv++)
        {
          Real sigma1 = 0.;
          Real sigma2 = 0.;

          /// calculate sigma1 with minmod
          if (sigma[2][iv] > 0. && sigma[1][iv] > 0.)
            sigma1 = std::min(sigma[2][iv], 2. * sigma[1][iv]);
          else if (sigma[2][iv] < 0. && sigma[1][iv] < 0.)
            sigma1 = std::max(sigma[2][iv], 2. * sigma[1][iv]);

          /// calculate sigma2 with minmod
          if (sigma[2][iv] > 0. && sigma[1][iv] > 0.)
            sigma2 = std::min(2. * sigma[2][iv], sigma[1][iv]);
          else if (sigma[2][iv] < 0. && sigma[1][iv] < 0.)
            sigma2 = std::max(2. * sigma[2][iv], sigma[1][iv]);

          /// calculate sigma with maxmod
          if (sigma1 > 0. && sigma2 > 0.)
            ugrad[iv](0) = std::max(sigma1, sigma2);
          else if (sigma1 < 0. && sigma2 < 0.)
            ugrad[iv](0) = std::min(sigma1, sigma2);
        }
      }
      break;

    default:
      mooseError("Unknown 1D TVD slope limiter scheme");
      break;
  }

  return ugrad;
}
