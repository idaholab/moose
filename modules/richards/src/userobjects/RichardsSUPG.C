/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Base class for Richards SUPG
//
#include "RichardsSUPG.h"

template<>
InputParameters validParams<RichardsSUPG>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Richards SUPG base class.  Override tauSUPG, etc");
  return params;
}

RichardsSUPG::RichardsSUPG(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters)
{}

void
RichardsSUPG::initialize()
{}

void
RichardsSUPG::execute()
{}

void RichardsSUPG::finalize()
{}
