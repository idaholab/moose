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

#ifndef NEMESIS_H
#define NEMESIS_H

// MOOSE includes
#include "AdvancedOutput.h"
#include "OversampleOutput.h"

// Forward declarations
class Nemesis;

// libMesh forward declarations
namespace libMesh
{
class Nemesis_IO;
}

template<>
InputParameters validParams<Nemesis>();

/**
 * Class for output data to the Nemesis format
 */
class Nemesis : public AdvancedOutput<OversampleOutput>
{
public:

  /**
   * Class constructor
   */
  Nemesis(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~Nemesis();

  /**
   * Overload the Output::output method, this is required for Nemesis
   * output due to the method utilized for outputing single/global parameters
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Sets up the libMesh::NemesisII_IO object used for outputting to the Nemesis format
   */
  virtual void initialSetup() override;

  /**
   * Creates a new NemesisII_IO output object for outputing a new mesh
   */
  virtual void meshChanged() override;


protected:

  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors() override;

  /**
   * Writes scalar AuxVariables to global output parameters
   */
  virtual void outputScalarVariables() override;

  /**
   * Returns the current filename, this method handles the -s000 suffix
   * common to NemesisII files.
   * @return A string containing the current filename to be written
   */
  virtual std::string filename() override;

  /// Pointer to the libMesh::NemesisII_IO object that performs the actual data output
  std::unique_ptr<Nemesis_IO> _nemesis_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /// Current output filename; utilized by filename() to create the proper suffix
  unsigned int _file_num;

private:

  /// Count of outputs per exodus file
  unsigned int _nemesis_num;

  /// Flag if the output has been initialized
  bool _nemesis_initialized;

};

#endif /* NEMESIS_H */
