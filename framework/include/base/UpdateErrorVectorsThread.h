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

#ifndef UPDATEERRORVECTORSTHREAD_H
#define UPDATEERRORVECTORSTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/elem_range.h"

class AuxiliarySystem;
class Adaptivity;

class UpdateErrorVectorsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  UpdateErrorVectorsThread(FEProblem & fe_problem, std::map<std::string, ErrorVector *> indicator_field_to_error_vector);

  // Splitting Constructor
  UpdateErrorVectorsThread(UpdateErrorVectorsThread & x, Threads::split split);

  virtual void onElement(const Elem *elem);

  void join(const UpdateErrorVectorsThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  std::map<std::string, ErrorVector *> _indicator_field_to_error_vector;
  AuxiliarySystem & _aux_sys;
  unsigned int _system_number;
  Adaptivity & _adaptivity;
  NumericVector<Number> & _solution;

  std::map<unsigned int, ErrorVector *> _indicator_field_number_to_error_vector;
};

#endif //UPDATEERRORVECTORSTHREAD_H
