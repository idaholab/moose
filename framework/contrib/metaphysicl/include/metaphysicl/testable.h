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
// $Id: core.h 37197 2013-02-21 05:49:09Z roystgnr $
//
//--------------------------------------------------------------------------



#ifndef METAPHYSICL_TESTABLE_H
#define METAPHYSICL_TESTABLE_H

namespace MetaPhysicL {

class safe_bool_base {
  public:
    typedef void (safe_bool_base::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
  protected:
 
    safe_bool_base() {}
    safe_bool_base(const safe_bool_base&) {}
    safe_bool_base& operator=(const safe_bool_base&) {return *this;}
    ~safe_bool_base() {}
};
 
// For testability without virtual function.
template <typename T=void> 
class safe_bool : private safe_bool_base {
  // private or protected inheritance is very important here as it
  // triggers the
  // access control violation in main.
  public:
    operator bool_type() const {
      return (static_cast<const T*>(this))->boolean_test()
        ? &safe_bool_base::this_type_does_not_support_comparisons : 0;
    }
  protected:
    ~safe_bool() {}
};
 
 
// For testability with a virtual function.
template<> 
class safe_bool<void> : private safe_bool_base {
  // private or protected inheritance is very important here as it
  // triggers the
  // access control violation in main.
  public:
    operator bool_type() const {
      safe_bool_base::this_type_does_not_support_comparisons();

      return boolean_test() 
        ? &safe_bool_base::this_type_does_not_support_comparisons : 0;
    }
  protected:
    virtual bool boolean_test() const=0;
    virtual ~safe_bool() {}
};
 
template <typename T> 
   inline bool operator==(const safe_bool<T>& lhs, bool b) {
      if (b)
      {
          if (lhs) return true;
          else return false;
      }
      else
      {
          if (lhs) return false;
          else return true;
      }
  }
 
template <typename T> 
   inline bool operator==(bool b, const safe_bool<T>& rhs) {
      if (b)
      {
          if (rhs) return true;
          else return false;
      }
      else
      {
          if (rhs) return false;
          else return true;
      }
  }
 
 
template <typename T, typename U> 
  inline void operator==(const safe_bool<T>& lhs,const safe_bool<U>& /*rhs*/) {
      lhs.this_type_does_not_support_comparisons();  
  }
 
template <typename T,typename U> 
  inline void operator!=(const safe_bool<T>& lhs,const safe_bool<U>& /*rhs*/) {
    lhs.this_type_does_not_support_comparisons();
  }

} // namespace MetaPhysicL

#endif // METAPHYSICL_TESTABLE_H
