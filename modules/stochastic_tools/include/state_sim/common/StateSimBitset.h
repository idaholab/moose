/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMBITSET_H
#define STATESIMBITSET_H

#include <vector>
#include <stdexcept>

/**
 * StateSimBitset is a bitset library to handle fast ID lookup for StateSim objects, only needed bitsest functions were defined.
 */
class StateSimBitset
{
public:
  /**
   * Copy construter.
   * @in_set - item to copy
   */
  StateSimBitset(const StateSimBitset & in_set);

  /**
   * This is the main construter for the user.
   * @param length - initial length of the bitset
   * @param to_state - state to move to for this transition.
   * @param move_desc - description of action that caused this transition.
   */
  StateSimBitset(int length);

  /**
   * Limited copy constructor.
   * @param length - initial length of the bitset
   * @param in_set - copy as much of this bitset as specified.
   */
  StateSimBitset(int length, const StateSimBitset & in_set);

  void setAll(bool value);
  int count() const
  {
    return _m_length;
  };
  void set(int idx, bool value);
  bool operator[](int i) const;
  StateSimBitset operator&(const StateSimBitset & in_set);
  bool hasCommonBits(const StateSimBitset & in_set) const;
  bool isEmpty() const;
  void resize(int length);

protected:
  std::vector<unsigned int> _m_array;
  int _m_length;

private:
  void checkOperand(const StateSimBitset & in_set) const;
};

#endif
