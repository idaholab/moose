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

#ifndef RANDGEN_H
#define RANDGEN_H

/// This class mostly exists to enable correct data[Store/Load] for restart/recovery by
/// enabling the unambiguous template overloading for those functions keyed
/// specifically to RandGen type.  This is necessary because c++ random generators
/// do not have a common base class.
template <typename T>
class RandGen : public T { };

#endif /* RANDGEN_H */
