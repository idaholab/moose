//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Euler2RGB.h"
#include "MathUtils.h"
#include "libmesh/utility.h"

/**
 * This function rotates a set of three Bunge Euler angles into
 * the standard Stereographic triangle, interpolates the RGB color
 * value based on a user selected reference sample direction, and
 * outputs an integer value representing the RGB tuplet usable for
 * plotting inverse pole figure colored grain maps.  The program
 * can accommodate any of the seven crystal systems.
 *
 *  Inputs:   1) Reference sample direction (sd) input as an integer between
 *               1 and 3.  Options are [100], [010], and [001] with [001]
 *               being the most common choice found in the literature. Input
 *               "1" to select [100], "2" to select [010], and "3" to select
 *               the [001] sample direction.
 *            2) Set of three Euler angles (phi1, PHI, phi2) in the Bunge
 *               notation (must be in radians)
 *            3) Phase number "phase" used to assign black color values to
 *               voids (phase = 0)
 *            4) Integer value of crystal class as defined by OIM Software:
 *                    "43" for cubic
 *                    "62" for hexagonal
 *                    "42" for tetragonal
 *                    "32" for trigonal
 *                    "22" for orthorhombic
 *                     "2" for monoclinic
 *                     "1" for triclinic
 *                     "0" for unindexed points (bad data point)
 *
 *  Output:  Integer value of RGB calculated from:
 *           RGB = red*256^2 + green*256 + blue (where red, green, and blue
 *           are integer values between 0 and 255)
 */
Point
euler2RGB(unsigned int sd, Real phi1, Real PHI, Real phi2, unsigned int phase, unsigned int sym)
{
  // Define Constants
  const Real pi = libMesh::pi;
  const Real pi_x2 = 2.0 * pi;
  const Real a = std::sqrt(3.0) / 2.0;

  // Preallocate and zero variables
  unsigned int index = 0;
  unsigned int nsym = 1;

  Real blue = 0.0;
  Real chi = 0.0;
  Real chi_min = 0.0;
  Real chi_max = 0.0;
  Real chi_max2 = 0.0;
  Real eta = 0.0;
  Real eta_min = 0.0;
  Real eta_max = 0.0;
  Real green = 0.0;
  Real maxRGB = 0.0;
  Real red = 0.0;

  unsigned int ref_dir[3] = {0, 0, 0};
  Real g[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  Real hkl[3] = {0.0, 0.0, 0.0};
  Real S[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  const Real(*SymOps)[3][3];

  Point RGB;

  // Assign reference sample direction
  switch (sd)
  {
    case 1: // 100
      ref_dir[0] = 1;
      ref_dir[1] = 0;
      ref_dir[2] = 0;
      break;

    case 2: // 010
      ref_dir[0] = 0;
      ref_dir[1] = 1;
      ref_dir[2] = 0;
      break;

    case 3: // 001
      ref_dir[0] = 0;
      ref_dir[1] = 0;
      ref_dir[2] = 1;
      break;
  };

  // Define symmetry operators for each of the seven crystal systems
  const Real SymOpsCubic[24][3][3] = {
      {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},    {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},
      {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}},    {{0, -1, 0}, {0, 0, 1}, {-1, 0, 0}},
      {{0, -1, 0}, {0, 0, -1}, {1, 0, 0}},  {{0, 1, 0}, {0, 0, -1}, {-1, 0, 0}},
      {{0, 0, -1}, {1, 0, 0}, {0, -1, 0}},  {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}},
      {{0, 0, 1}, {-1, 0, 0}, {0, -1, 0}},  {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
      {{-1, 0, 0}, {0, -1, 0}, {0, 0, 1}},  {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}},
      {{0, 0, -1}, {0, -1, 0}, {-1, 0, 0}}, {{0, 0, 1}, {0, -1, 0}, {1, 0, 0}},
      {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},   {{0, 0, -1}, {0, 1, 0}, {1, 0, 0}},
      {{-1, 0, 0}, {0, 0, -1}, {0, -1, 0}}, {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}},
      {{1, 0, 0}, {0, 0, 1}, {0, -1, 0}},   {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
      {{0, -1, 0}, {-1, 0, 0}, {0, 0, -1}}, {{0, 1, 0}, {-1, 0, 0}, {0, 0, -1}},
      {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},   {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}}};

  const Real SymOpsHexagonal[12][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                          {{-0.5, a, 0}, {-a, -0.5, 0}, {0, 0, 1}},
                                          {{-0.5, -a, 0}, {a, -0.5, 0}, {0, 0, 1}},
                                          {{0.5, a, 0}, {-a, 0.5, 0}, {0, 0, 1}},
                                          {{-1, 0, 0}, {0, -1, 0}, {0, 0, 1}},
                                          {{0.5, -a, 0}, {a, 0.5, 0}, {0, 0, 1}},
                                          {{-0.5, -a, 0}, {-a, 0.5, 0}, {0, 0, -1}},
                                          {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}},
                                          {{-0.5, a, 0}, {a, 0.5, 0}, {0, 0, -1}},
                                          {{0.5, a, 0}, {a, -0.5, 0}, {0, 0, -1}},
                                          {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
                                          {{0.5, -a, 0}, {-a, -0.5, 0}, {0, 0, -1}}};

  const Real SymOpsTetragonal[8][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                          {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
                                          {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}},
                                          {{-1, 0, 0}, {0, -1, 0}, {0, 0, 1}},
                                          {{0, 1, 0}, {-1, 0, 0}, {0, 0, 1}},
                                          {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
                                          {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},
                                          {{0, -1, 0}, {-1, 0, 0}, {0, 0, -1}}};

  const Real SymOpsTrigonal[6][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                        {{-0.5, a, 0}, {-a, -0.5, 0}, {0, 0, 1}},
                                        {{-0.5, -a, 0}, {a, -0.5, 0}, {0, 0, 1}},
                                        {{0.5, a, 0}, {a, -0.5, 0}, {0, 0, -1}},
                                        {{-1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                        {{0.5, -a, 0}, {-a, -0.5, 0}, {0, 0, -1}}};

  const Real SymOpsOrthorhombic[4][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                            {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
                                            {{1, 0, 0}, {0, -1, 0}, {0, 0, 1}},
                                            {{-1, 0, 0}, {0, -1, 0}, {0, 0, 1}}};

  const Real SymOpsMonoclinic[2][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                                          {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}}};

  const Real SymOpsTriclinic[1][3][3] = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};

  // Assign parameters based on crystal class (sym)
  // Load cubic parameters (class 432)
  if (sym == 43)
  {
    nsym = 24;
    SymOps = SymOpsCubic;
    eta_min = 0 * (pi / 180);
    eta_max = 45 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = std::acos(std::sqrt(1.0 / (2.0 + (Utility::pow<2>(std::tan(eta_max))))));
  }

  //  Load hexagonal parameters (class 622)
  else if (sym == 62)
  {
    nsym = 12;
    SymOps = SymOpsHexagonal;
    eta_min = 0 * (pi / 180);
    eta_max = 30 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  //  Load tetragonal parameters (class 422)
  else if (sym == 42)
  {
    nsym = 8;
    SymOps = SymOpsTetragonal;
    eta_min = 0 * (pi / 180);
    eta_max = 45 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  //  Load trigonal parameters (class 32)
  else if (sym == 32)
  {
    nsym = 6;
    SymOps = SymOpsTrigonal;
    eta_min = 0 * (pi / 180);
    eta_max = 60 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  //  Load orthorhombic parameters (class 22)
  else if (sym == 22)
  {
    nsym = 4;
    SymOps = SymOpsOrthorhombic;
    eta_min = 0 * (pi / 180);
    eta_max = 90 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  //  Load monoclinic parameters (class 2)
  else if (sym == 2)
  {
    nsym = 2;
    SymOps = SymOpsMonoclinic;
    eta_min = 0 * (pi / 180);
    eta_max = 180 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  //  Load triclinic parameters (class 1)
  else if (sym == 1)
  {
    nsym = 1;
    SymOps = SymOpsTriclinic;
    eta_min = 0 * (pi / 180);
    eta_max = 360 * (pi / 180);
    chi_min = 0 * (pi / 180);
    chi_max = pi / 2;
  }

  // Accomodate non-conforming (bad) data points
  else
  {
    nsym = 0;
  }

  // Start of main routine //
  // Assign black RGB values for bad data points (nsym = 0) or voids (phase = 0)
  if (nsym == 0 || phase == 0)
    RGB = 0;

  // Assign black RGB value for Euler angles outside of allowable range
  else if (phi1 > pi_x2 || PHI > pi || phi2 > pi_x2)
    RGB = 0;

  //  Routine for valid set of Euler angles
  else
  {
    // Construct 3X3 orientation matrix
    g[0][0] = std::cos(phi1) * std::cos(phi2) - std::sin(phi1) * std::cos(PHI) * std::sin(phi2);
    g[0][1] = std::sin(phi1) * std::cos(phi2) + std::cos(phi1) * std::cos(PHI) * std::sin(phi2);
    g[0][2] = std::sin(phi2) * std::sin(PHI);
    g[1][0] = -std::cos(phi1) * std::sin(phi2) - std::sin(phi1) * std::cos(PHI) * std::cos(phi2);
    g[1][1] = -std::sin(phi1) * std::sin(phi2) + std::cos(phi1) * std::cos(PHI) * std::cos(phi2);
    g[1][2] = std::cos(phi2) * std::sin(PHI);
    g[2][0] = std::sin(phi1) * std::sin(PHI);
    g[2][1] = -std::cos(phi1) * std::sin(PHI);
    g[2][2] = std::cos(PHI);

    // Loop to sort Euler angles into standard stereographic triangle (SST)
    index = 0;
    while (index < nsym)
    {
      // Form orientation matrix
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
        {
          S[i][j] = 0.0;
          for (unsigned int k = 0; k < 3; ++k)
            S[i][j] += SymOps[index][i][k] * g[k][j];
        }

      // Multiple orientation matrix by reference sample direction
      for (unsigned int i = 0; i < 3; ++i)
      {
        hkl[i] = 0;
        for (unsigned int j = 0; j < 3; ++j)
          hkl[i] += S[i][j] * ref_dir[j];
      }

      // Convert to spherical coordinates (ignore "r" variable since r=1)
      eta = std::abs(std::atan2(hkl[1], hkl[0]));
      chi = std::acos(std::abs(hkl[2]));

      // Continue if eta and chi values are within the SST
      if (eta >= eta_min && eta < eta_max && chi >= chi_min && chi < chi_max)
        break;

      // Increment to next symmetry operator if not in SST
      else
        index++;

      // Check for solution
      mooseAssert(index != nsym, "Euler2RGB failed to map the supplied Euler angle into the SST!");
    }

    //  Adjust maximum chi value to ensure it falls within the SST (cubic materials only)
    if (sym == 43)
      chi_max2 = std::acos(std::sqrt(1.0 / (2.0 + (Utility::pow<2>(std::tan(eta))))));
    else
      chi_max2 = pi / 2;

    // Calculate the RGB color values and make adjustments to maximize colorspace
    red = std::abs(1.0 - (chi / chi_max2));
    blue = std::abs((eta - eta_min) / (eta_max - eta_min));
    green = 1.0 - blue;

    blue = blue * (chi / chi_max2);
    green = green * (chi / chi_max2);

    // Check for negative RGB values before taking square root
    mooseAssert(red >= 0 || green >= 0 || blue >= 0, "RGB component values must be positive!");

    RGB(0) = std::sqrt(red);
    RGB(1) = std::sqrt(green);
    RGB(2) = std::sqrt(blue);

    // Find maximum value of red, green, or blue
    maxRGB = std::max({RGB(0), RGB(1), RGB(2)});

    // Normalize position of SST center point
    RGB /= maxRGB;
  }

  return RGB;
}
