/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#pragma once

#include "GeneralUserObject.h"
#include <vector>


/**
 * Read a file to provide initial dislocation densities to grains
 **/
class DislocationDensityFileReader : public GeneralUserObject
{
public:
  DislocationDensityFileReader(const InputParameters & parameters);

  static InputParameters validParams();

  /// return the dislocation density
  virtual const Real & getDensity(unsigned int) const;

  /// get the grain ID
  virtual unsigned int getGrainNum() const;

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

private:
  /// read the file for data
  void readFile();

  /// file name
  FileName _file_name;

  /// number of header lines to skip in the file
  unsigned int _lines_to_skip;

  /// dislocation densities
  std::vector<Real> _densities;
};
