// This seems redundant

#if 0

#ifndef TESTEBSDAUX_H
#define TESTEBSDAUX_H

#include "AuxKernel.h"
#include "EBSDReader.h"

// Forward Declarations
class TestEBSDAux;

template<>
InputParameters validParams<TestEBSDAux>();

/**
 * This kernel tests the EBSDReader GeneralUserObject by using it to
 * set up an Aux variable.
 */
class TestEBSDAux : public AuxKernel
{
public:
  TestEBSDAux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();
  const EBSDReader & _ebsd_reader;

  /// String and associated MooseEnum that stores the type of data
  /// this AuxKernel extracts.
  std::string _data_name;
  MooseEnum _data_type;
};

#endif //TESTEBSDAUX_H

#endif
