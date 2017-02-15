/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimBitset.h"
#include "MooseApp.h"
#include <vector>

const int NUMBITS = 32;
const int NUMBITS_MIN1 = 31;

StateSimBitset::StateSimBitset(int length)
  : _m_array(),
    _m_length(length)
{
  mooseAssert((length > 0), "StateSimBitset - length must be >= 0");

  unsigned long slots = (unsigned long)((length + NUMBITS_MIN1) / NUMBITS);
  _m_array.resize(slots, 0);
}

StateSimBitset::StateSimBitset(const StateSimBitset & in_set)
  : _m_array(in_set._m_array),
    _m_length(in_set._m_length)
{
}

StateSimBitset::StateSimBitset(int length, const StateSimBitset & in_set)
  : _m_array(),
    _m_length(length)
{
  if (in_set._m_length < length)
  {
    _m_length = in_set._m_length;
  }

  int slots = (int)((_m_length + NUMBITS_MIN1) / NUMBITS);
  int in_slots = (int)((in_set._m_length + NUMBITS_MIN1) / NUMBITS);

  for (int i = 0; (i < slots) && (i < in_slots); i++)
  {
    _m_array[i] = in_set._m_array[i];
  }
}

void
StateSimBitset::resize(int length)
{
  if (this->_m_length < length)
  {
    unsigned int numints = (length + 31) / 32;
    _m_array.resize(numints, 0);
  }
  else //clear out any bits over the new length.
  {
    for (int i = length - 1; i < _m_length; i++)
    {
      this->set(i, false);
    }
  }
  _m_length = length;
}

void
StateSimBitset::setAll(bool value)
{
  int set_val = 0;
  if (value)
  {
    set_val = 0xFFFF;
  }

  for (unsigned int i = 0; i < _m_array.size(); i++)
  {
    _m_array[i] = set_val;
  }
}

void
StateSimBitset::set(int idx, bool value)
{
  mooseAssert(!(idx < 0 || idx >= _m_length), "SateSimBitset set() - idx not in bitset range");

  if (value)
  {
    _m_array[idx >> 5] |= (1 << (idx & NUMBITS_MIN1));
  }
  else
  {
    _m_array[idx >> 5] &= ~(1 << (idx & NUMBITS_MIN1));
  }
}

bool
    StateSimBitset::
    operator[](int i) const
{
  mooseAssert(!(i < 0 || i >= _m_length), "SateSimBitset operator[] - i not in bitset range");
  return (_m_array[i >> 5] & (1 << (i & NUMBITS_MIN1))) != 0;
}

StateSimBitset StateSimBitset::operator&(const StateSimBitset & in_set)
{
  checkOperand(in_set);

  int ints = (_m_length + NUMBITS_MIN1) / NUMBITS;
  for (int i = 0; i < ints; i++)
  {
    _m_array[i] &= in_set._m_array[i];
  }

  return *this;
}

bool
StateSimBitset::hasCommonBits(const StateSimBitset & in_set) const
{
  if (this->_m_length < in_set._m_length)
  {
    //StateSimBitset * other = &this;
    StateSimBitset copied(this->_m_length, in_set);
    copied = copied & *this;
    return !copied.isEmpty();
  }
  else
  {
    //StateSimBitset * other = &in_set;
    StateSimBitset copied(in_set._m_length, *this);
    copied = copied & in_set;
    return !copied.isEmpty();
  }
}

bool
StateSimBitset::isEmpty() const
{
  for (unsigned int i = 0; i < _m_array.size(); i++)
  {
    if (_m_array[i] != 0)
      return false;
  }

  return true;
}

void
StateSimBitset::checkOperand(const StateSimBitset & operand) const
{
  mooseAssert((operand._m_length != this->_m_length), "SateSimBitset checkOperand() - operand length not equal to this length.");
}
