//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// infix_iterator.h

// Lifted from Jerry Coffin's 's prefix_ostream_iterator, no copyright or license
#ifndef INFIXITERATOR_H
#define INFIXITERATOR_H

#include <ostream>
#include <iterator>

template <class T, class charT = char, class traits = std::char_traits<charT>>
class infix_ostream_iterator
    : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  std::basic_ostream<charT, traits> * os;
  charT const * delimiter;
  bool first_elem;

public:
  typedef charT char_type;
  typedef traits traits_type;
  typedef std::basic_ostream<charT, traits> ostream_type;
  infix_ostream_iterator(ostream_type & s) : os(&s), delimiter(0), first_elem(true) {}
  infix_ostream_iterator(ostream_type & s, charT const * d, bool first = true)
    : os(&s), delimiter(d), first_elem(first)
  {
  }
  infix_ostream_iterator<T, charT, traits> & operator=(T const & item)
  {
    // Here's the only real change from ostream_iterator:
    // Normally, the '*os << item;' would come before the 'if'.
    if (!first_elem && delimiter != 0)
      *os << delimiter;
    *os << item;
    first_elem = false;
    return *this;
  }
  infix_ostream_iterator<T, charT, traits> & operator*() { return *this; }
  infix_ostream_iterator<T, charT, traits> & operator++() { return *this; }
  infix_ostream_iterator<T, charT, traits> & operator++(int) { return *this; }
};

#endif
