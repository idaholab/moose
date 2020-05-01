//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#if defined(__GNUC__) && defined(LIBMESH_HAVE_EXTERNAL_BOOST)
#pragma GCC system_header
#include <boost/math/distributions.hpp>
#endif
#if defined(__clang__) && defined(LIBMESH_HAVE_EXTERNAL_BOOST)
#pragma clang system_header
#include <boost/math/distributions.hpp>
#endif
