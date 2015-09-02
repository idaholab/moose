//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// MetaPhysicL - A metaprogramming library for physics calculations
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
//
// $Id$
//
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#ifndef METAPHYSICL_CAST_H
#define METAPHYSICL_CAST_H

// MetaPhysicL
#include "metaphysicl/metaphysicl_asserts.h"

// C++
#include <typeinfo> // std::bad_cast

// cast_ref and cast_ptr do a dynamic cast and assert the result, if
// we're debugging, but they just do a faster static cast if we're
// built with NDEBUG.
//
// Use these casts when you're certain that a cast will succeed in
// correct code but you want to be able to double-check.
template <typename Tnew, typename Told>
inline Tnew cast_ref(Told & oldvar)
{
#if !defined(NDEBUG)
  try
    {
      Tnew newvar = dynamic_cast<Tnew>(oldvar);
      return newvar;
    }
  catch (std::bad_cast)
    {
      std::cerr << "Failed to convert " << typeid(Told).name()
                << " reference to " << typeid(Tnew).name()
                << std::endl;
      std::cerr << "The " << typeid(Told).name()
                << " appears to be a "
                << typeid(*(&oldvar)).name() << std::endl;
      metaphysicl_error();
    }
#else
  return(static_cast<Tnew>(oldvar));
#endif
}



// We use two different function names to avoid an odd overloading
// ambiguity bug with icc 10.1.008
template <typename Tnew, typename Told>
inline Tnew cast_ptr (Told * oldvar)
{
#if !defined(NDEBUG)
  Tnew newvar = dynamic_cast<Tnew>(oldvar);
  if (!newvar)
    {
      std::cerr << "Failed to convert " << typeid(Told).name()
                << " pointer to " << typeid(Tnew).name()
                << std::endl;
      std::cerr << "The " << typeid(Told).name()
                << " appears to be a "
                << typeid(*oldvar).name() << std::endl;
      metaphysicl_error();
    }
  return newvar;
#else
  return(static_cast<Tnew>(oldvar));
#endif
}

#endif // METAPHYSICL_CAST_H
