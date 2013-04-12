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

#ifndef NODALVOLUMEFRACTION_H
#define NODALVOLUMEFRACTION_H

#include "NodalFloodCount.h"

//Forward Declarations
class NodalVolumeFraction;

template<>
InputParameters validParams<NodalVolumeFraction>();

class NodalVolumeFraction : public NodalFloodCount
{
public:
  NodalVolumeFraction(const std::string & name, InputParameters parameters);
  ~NodalVolumeFraction();

  virtual void finalize();

  Real getValue();
  
  void calculateBubbleFraction();

  void calculateAvramiInformation(const std::string & file_name);
  
protected:
  
private:

  Real & _mesh_volume;
  Real _volume_fraction;
  Real _equil_fraction;

/**
   * The filename and filehandle used if bubble volumes are being recorded to a file.
   */
//  std::string _bubble_fraction_file_name;
  // std::ofstream _bubble_fraction_file_handle;

  std::string _Avrami_file_name;
  std::ofstream _Avrami_file_handle;
};

#endif //NODALVOLUMEFRACTION_H

