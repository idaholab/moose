/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PACKEDRANGEOVERFLOW_H
#define PACKEDRANGEOVERFLOW_H

#include "GeneralUserObject.h"

class PackedRangeOverflow;

template<>
InputParameters validParams<PackedRangeOverflow>();

/**
 * Debug the packed range allgather method
 */
class PackedRangeOverflow : public GeneralUserObject
{
public:
  PackedRangeOverflow(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  unsigned int _buffer_size;
};

#endif //PACKEDRANGEOVERFLOW_H
