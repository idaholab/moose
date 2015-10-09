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

#ifndef METAPHYSICL_ASSERTS_H
#define METAPHYSICL_ASSERTS_H

// C++
#include <iostream>
#include <iomanip>

// MetaPhysicL
#include "metaphysicl/metaphysicl_exceptions.h"

#define metaphysicl_here()     do { std::cerr << __FILE__ << ", line " << __LINE__ << ", compiled " << __DATE__ << " at " << __TIME__ << std::endl; } while (0)

// The metaphysicl_assert() macro acts like C's assert(), but throws a
// metaphysicl_error() (including stack trace, etc) instead of just exiting
#ifdef NDEBUG
#define metaphysicl_assert(asserted)  ((void) 0)
#define metaphysicl_assert_msg(asserted, msg)  ((void) 0)
#define metaphysicl_assert_equal_to(expr1,expr2)  ((void) 0)
#define metaphysicl_assert_not_equal_to(expr1,expr2)  ((void) 0)
#define metaphysicl_assert_less(expr1,expr2)  ((void) 0)
#define metaphysicl_assert_greater(expr1,expr2)  ((void) 0)
#define metaphysicl_assert_less_equal(expr1,expr2)  ((void) 0)
#define metaphysicl_assert_greater_equal(expr1,expr2)  ((void) 0)
#else
#define metaphysicl_assert(asserted)  do { if (!(asserted)) { std::cerr << "Assertion `" #asserted "' failed." << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_equal_to(expr1,expr2)  do { if (!(expr1 == expr2)) { std::cerr << "Assertion `" #expr1 " == " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_not_equal_to(expr1,expr2)  do { if (!(expr1 != expr2)) { std::cerr << "Assertion `" #expr1 " != " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_less(expr1,expr2)  do { if (!(expr1 < expr2)) { std::cerr << "Assertion `" #expr1 " < " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_greater(expr1,expr2)  do { if (!(expr1 > expr2)) { std::cerr << "Assertion `" #expr1 " > " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_less_equal(expr1,expr2)  do { if (!(expr1 <= expr2)) { std::cerr << "Assertion `" #expr1 " <= " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#define metaphysicl_assert_greater_equal(expr1,expr2)  do { if (!(expr1 >= expr2)) { std::cerr << "Assertion `" #expr1 " >= " #expr2 "' failed.\n" #expr1 " = " << (expr1) << "\n" #expr2 " = " << (expr2) << std::endl; metaphysicl_error(); } } while(0)
#endif


#define metaphysicl_error()    do { metaphysicl_here(); METAPHYSICL_THROW(MetaPhysicL::LogicError()); } while(0)
#define metaphysicl_not_implemented()    do { metaphysicl_here(); METAPHYSICL_THROW(MetaPhysicL::NotImplemented()); } while(0)
#define metaphysicl_file_error(filename)    do { metaphysicl_here(); METAPHYSICL_THROW(MetaPhysicL::FileError(filename)); } while(0)

#endif // METAPHYSICL_ASSERTS_H
