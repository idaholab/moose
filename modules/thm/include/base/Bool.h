#ifndef BOOL_H
#define BOOL_H

/**
 * A wrapper for the C++ boolean type which can be stored in vectors
 * in the same way as other C++ types.
 */
struct Bool
{
  // If an 'uninitialized' Bool is created, set _value to false
  Bool() : _value(false) {}
  Bool(bool b) : _value(b) {}

  // This allows a lot of stuff that works for bool to also work for
  // Bool, like cout.
  operator bool &() { return _value; }
  operator const bool &() const { return _value; }

  Bool & operator=(const bool & other)
  {
    _value = other;
    return *this;
  }

  bool _value;
};

#endif
