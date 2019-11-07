//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrimeProductUserObject.h"
#include <unistd.h>

registerMooseObject("MooseTestApp", PrimeProductUserObject);

InputParameters
PrimeProductUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

PrimeProductUserObject::PrimeProductUserObject(const InputParameters & params)
  : ThreadedGeneralUserObject(params)
{
  if (libMesh::n_threads() > 20)
    mooseError("This object works only with up to 20 threads. If you need more, add more prime "
               "numbers into `prime` variable in the execute method.");
}

void
PrimeProductUserObject::initialize()
{
}

void
PrimeProductUserObject::execute()
{
  // first 20 prime numbers
  static unsigned int primes[] = {2,  3,  5,  7,  11, 13, 17, 19, 23, 29,
                                  31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
  _product = primes[_tid];
}

void
PrimeProductUserObject::finalize()
{
}

void
PrimeProductUserObject::threadJoin(const UserObject & uo)
{
  const PrimeProductUserObject & pcuo = dynamic_cast<const PrimeProductUserObject &>(uo);
  _product *= pcuo._product;
}
