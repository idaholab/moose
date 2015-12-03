/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TESTEBSDAUX_H
#define TESTEBSDAUX_H

#include "AuxKernel.h"
#include "EBSDReader.h"

//Forward Declarations
class TestEBSDAux;

template<>
InputParameters validParams<TestEBSDAux>();

/**
 * This kernel tests the EBSDReader GeneralUserObject by using it to
 * set up an Aux variable.
 */
class TestEBSDAux : public AuxKernel, EBSDAccessFunctors
{
public:
  TestEBSDAux(const InputParameters & parameters);
  ~TestEBSDAux();

protected:
  virtual Real computeValue();
  const EBSDReader & _ebsd_reader;

  /// String and associated MooseEnum that stores the type of data
  /// this AuxKernel extracts.
  MooseEnum _data_name;

  /// Accessor functor to fetch the selected data field form the EBSD data point
  MooseSharedPointer<EBSDPointDataFunctor> _val;
};

#endif //TESTEBSDAUX_H
