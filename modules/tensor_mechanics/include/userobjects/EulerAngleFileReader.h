/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEFILEREADER_H
#define EULERANGLEFILEREADER_H

#include "EulerAngleProvider.h"
#include <vector>

// Forward declaration
class EulerAngleFileReader;

template <>
InputParameters validParams<EulerAngleFileReader>();

/**
 * Read a set of Euler angles from a file
 */
class EulerAngleFileReader : public EulerAngleProvider
{
public:
  EulerAngleFileReader(const InputParameters & parameters);

  virtual const EulerAngles & getEulerAngles(unsigned int) const;
  virtual unsigned int getGrainNum() const;

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

protected:
  void readFile();

  FileName _file_name;
  std::vector<EulerAngles> _angles;
};

#endif // EULERANGLEFILEREADER_H
