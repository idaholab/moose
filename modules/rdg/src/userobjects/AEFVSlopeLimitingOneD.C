//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVSlopeLimitingOneD.h"

registerMooseObject("RdgApp", AEFVSlopeLimitingOneD);

InputParameters
AEFVSlopeLimitingOneD::validParams()
{
  InputParameters params = SlopeLimitingBase::validParams();
  params.addClassDescription("One-dimensional slope limiting to get the limited slope of cell "
                             "average variable for the advection equation using a cell-centered "
                             "finite volume method.");
  params.addRequiredCoupledVar("u", "constant monomial variable");
  MooseEnum scheme("none minmod mc superbee", "none");
  params.addParam<MooseEnum>("scheme", scheme, "TVD-type slope limiting scheme");
  return params;
}

AEFVSlopeLimitingOneD::AEFVSlopeLimitingOneD(const InputParameters & parameters)
  : SlopeLimitingBase(parameters), _u(getVar("u", 0)), _scheme(getParam<MooseEnum>("scheme"))
{
}

std::vector<RealGradient>
AEFVSlopeLimitingOneD::limitElementSlope() const
{
  // you should know how many equations you are solving and assign this number
  // e.g. = 1 (for the advection equation)
  unsigned int nvars = 1;

  const Elem * elem = _current_elem;

  // index of conserved variable
  unsigned int iv;

  // index of sides around an element
  unsigned int is;

  // index of the neighbor element
  unsigned int in;

  // number of sides surrounding an element = 2 in 1D
  unsigned int nside = elem->n_sides();

  // number of reconstruction stencils = 3 in 1D
  unsigned int nsten = nside + 1;

  // vector for the gradients of primitive variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));

  // array to store center coordinates of this cell and its neighbor cells
  std::vector<Real> xc(nsten, 0.);

  // the first always stores the current cell
  xc[0] = elem->vertex_average()(0);

  // array for the cell-average values in the current cell and its neighbors
  std::vector<std::vector<Real>> ucell(nsten, std::vector<Real>(nvars, 0.));

  // central slope:
  //                  u_{i+1} - u {i-1}
  //                  -----------------
  //                  x_{i+1} - x_{i-1}

  // left-side slope:
  //                  u_{i} - u {i-1}
  //                  ---------------
  //                  x_{i} - x_{i-1}

  // right-side slope:
  //                  u_{i+1} - u {i}
  //                  ---------------
  //                  x_{i+1} - x_{i}

  // array to store the central and one-sided slopes, where the first should be central slope
  std::vector<std::vector<Real>> sigma(nsten, std::vector<Real>(nvars, 0.));

  // get the cell-average variable in the central cell
  if (_is_implicit)
    ucell[0][0] = _u->getElementalValue(elem);
  else
    ucell[0][0] = _u->getElementalValueOld(elem);

  // a flag to indicate the boundary side of the current cell

  unsigned int bflag = 0;

  // loop over the sides to compute the one-sided slopes

  for (is = 0; is < nside; is++)
  {
    in = is + 1;

    // when the current element is an internal cell
    if (elem->neighbor_ptr(is) != NULL)
    {
      const Elem * neig = elem->neighbor_ptr(is);
      if (this->hasBlocks(neig->subdomain_id()))
      {
        xc[in] = neig->vertex_average()(0);

        // get the cell-average variable in this neighbor cell
        if (_is_implicit)
          ucell[in][0] = _u->getElementalValue(neig);
        else
          ucell[in][0] = _u->getElementalValueOld(neig);

        // calculate the one-sided slopes of primitive variables

        for (iv = 0; iv < nvars; iv++)
          sigma[in][iv] = (ucell[0][iv] - ucell[in][iv]) / (xc[0] - xc[in]);
      }
      else
        bflag = in;
    }
    // when the current element is at the boundary,
    // we choose not to construct the slope in 1D just for convenience.
    else
    {
      bflag = in;
    }
  }

  // calculate the slope of the current element
  // according to the choice of the slope limiter algorithm

  switch (_scheme)
  {
    // first-order, zero slope
    case 0:
      break;

    // ==============
    // minmod limiter
    // ==============
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

    // ===========================================
    // MC (monotonized central-difference limiter)
    // ===========================================
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

    // ================
    // Superbee limiter
    // ================
    case 3:

      if (bflag == 0)
      {
        for (iv = 0; iv < nvars; iv++)
        {
          Real sigma1 = 0.;
          Real sigma2 = 0.;

          // calculate sigma1 with minmod
          if (sigma[2][iv] > 0. && sigma[1][iv] > 0.)
            sigma1 = std::min(sigma[2][iv], 2. * sigma[1][iv]);
          else if (sigma[2][iv] < 0. && sigma[1][iv] < 0.)
            sigma1 = std::max(sigma[2][iv], 2. * sigma[1][iv]);

          // calculate sigma2 with minmod
          if (sigma[2][iv] > 0. && sigma[1][iv] > 0.)
            sigma2 = std::min(2. * sigma[2][iv], sigma[1][iv]);
          else if (sigma[2][iv] < 0. && sigma[1][iv] < 0.)
            sigma2 = std::max(2. * sigma[2][iv], sigma[1][iv]);

          // calculate sigma with maxmod
          if (sigma1 > 0. && sigma2 > 0.)
            ugrad[iv](0) = std::max(sigma1, sigma2);
          else if (sigma1 < 0. && sigma2 < 0.)
            ugrad[iv](0) = std::min(sigma1, sigma2);
        }
      }
      break;

    default:
      mooseError("Unknown 1D TVD-type slope limiter scheme");
      break;
  }

  return ugrad;
}
