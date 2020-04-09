//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/vector_value.h"
#include "libmesh/point.h"
#include "libmesh/parameters.h"

/**
 *   Manufactured solution for advection-diffusion problem-1.
 */
Number ManSol4ADR1(
    const Point & p, Real & A0, Real & B0, Real & C0, Real & omega, Real & t, bool & is_transient);
Number ManSol4ADR1src(const Point & p,
                      Real & A0,
                      Real & B0,
                      Real & C0,
                      Real & Au,
                      Real & Bu,
                      Real & Cu,
                      Real & Av,
                      Real & Bv,
                      Real & Cv,
                      Real & Ak,
                      Real & Bk,
                      Real & Ck,
                      Real & omega,
                      Real & t,
                      bool & is_transient);
Number
ManSol4ADR1exv(const Point & p, const Parameters &, const std::string &, const std::string &);
Gradient
ManSol4ADR1exd(const Point & p, const Parameters &, const std::string &, const std::string &);

/**
 *   Manufactured solution for advection-diffusion problem-2.
 */
Number ManSol4ADR2(const Point & p, Real & A0, Real & B0, Real & C0, Real & omega, Real & t);
Number ManSol4ADR2src(const Point & p,
                      Real & A0,
                      Real & B0,
                      Real & C0,
                      Real & Au,
                      Real & Bu,
                      Real & Cu,
                      Real & Av,
                      Real & Bv,
                      Real & Cv,
                      Real & Ak,
                      Real & Bk,
                      Real & Ck,
                      Real & omega,
                      Real & t);
Number
ManSol4ADR2exv(const Point & p, const Parameters &, const std::string &, const std::string &);
Gradient
ManSol4ADR2exd(const Point & p, const Parameters &, const std::string &, const std::string &);

Number ManSolzeroV(const Point & p, const Parameters &, const std::string &, const std::string &);
Gradient ManSolzeroG(const Point & p, const Parameters &, const std::string &, const std::string &);
