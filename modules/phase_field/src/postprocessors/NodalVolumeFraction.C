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

#include "NodalVolumeFraction.h"
#include <cmath>

template<>
InputParameters validParams<NodalVolumeFraction>()
{
  InputParameters params = validParams<NodalFloodCount>();
  params.addRequiredParam<std::string>("mesh_volume", "Postprocessor from which to get mesh volume");
  // params.addParam<FileName>("bubble_fraction_file", "filename for bubble fraction data output");
  params.addParam<FileName>("Avrami_file", "filename for Avrami analysis info (ln time and Avrami)");
  params.addParam<Real>("equil_fraction", -1.0, "Equilibrium volume fraction of 2nd phase for Avrami analysis");
    
  return params;
}

NodalVolumeFraction::NodalVolumeFraction(const std::string & name, InputParameters parameters) :
    NodalFloodCount(name, parameters),
    _mesh_volume(getPostprocessorValue(getParam<std::string>("mesh_volume"))),
    _equil_fraction(getParam<Real>("equil_fraction")),
    // _bubble_fraction_file_name(parameters.isParamValid("bubble_fraction_file") ? getParam<FileName>("bubble_fraction_file") : ""),
    _Avrami_file_name(parameters.isParamValid("Avrami_file") ? getParam<FileName>("Avrami_file") : "")
{
  if (_Avrami_file_name != "" && _equil_fraction < 0.0)
    mooseError("please supply an equilibrium fraction of 2nd phase for Avrami analysis (NodalVolumeFraction).");
}

NodalVolumeFraction::~NodalVolumeFraction()
{
  if (_bubble_volume_file_handle.is_open())
    _bubble_volume_file_handle.close();

  if(_Avrami_file_handle.is_open())
    _Avrami_file_handle.close();
}

void
NodalVolumeFraction::finalize()
{
  NodalFloodCount::finalize();

  // If the bubble volume calculation wasn't done yet, do now.
  if (_bubble_volume_file_name == "")
    NodalFloodCount::calculateBubbleVolumes(_bubble_volume_file_name);

  //if(_bubble_fraction_file_name != "")
    calculateBubbleFraction();

    if (_Avrami_file_name != "")
      calculateAvramiInformation(_Avrami_file_name);
}

Real
NodalVolumeFraction::getValue()
{
  return _volume_fraction;
}

void
NodalVolumeFraction::calculateBubbleFraction()
{
  Real volume;
//sum the values in the vector to get total volume
  for (std::vector<Real>::const_iterator it = _all_bubble_volumes.begin(); it != _all_bubble_volumes.end(); ++it)
    volume += *it;
  
  _volume_fraction = volume/_mesh_volume; 
}

void
NodalVolumeFraction::calculateAvramiInformation(const std::string & file_name)
{
  Real ln_time;
  Real Avrami;

  ln_time = std::log(_fe_problem.time());

  Avrami = log( log(1.0/(1.0 - (_volume_fraction/_equil_fraction) ) ) );

  if (libMesh::processor_id() == 0)
  {
    if (!_Avrami_file_handle.is_open())
    {
      MooseUtils::checkFileWriteable(file_name);
      _Avrami_file_handle.open(file_name.c_str());
    }

    _Avrami_file_handle << _fe_problem.timeStep() << ", " << std::scientific << std::setprecision(6) << _fe_problem.time();
    _Avrami_file_handle << ", " << std::scientific << std::setprecision(6) << ln_time << ", " << Avrami;
    _Avrami_file_handle << std::endl;
 }
}


