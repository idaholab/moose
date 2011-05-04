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

#include "Conversion.h"
#include <map>
#include <algorithm>

namespace Moose {

  std::map<std::string, TimeSteppingScheme> timesteppingscheme_type_to_enum;
  std::map<std::string, ExecFlagType> execstore_type_to_enum;

  void initTimeSteppingMap()
  {
    if (timesteppingscheme_type_to_enum.empty())
    {
      timesteppingscheme_type_to_enum["BACKWARD-EULER"] = IMPLICIT_EULER;
      timesteppingscheme_type_to_enum["IMPLICIT-EULER"] = IMPLICIT_EULER;
      timesteppingscheme_type_to_enum["CRANK-NICOLSON"] = CRANK_NICOLSON;
      timesteppingscheme_type_to_enum["BDF2"]           = BDF2;
    }
  }

  void initExecStoreType()
  {
    if (execstore_type_to_enum.empty())
    {
      execstore_type_to_enum["INITIAL"]  = EXEC_INITIAL;
      execstore_type_to_enum["RESIDUAL"] = EXEC_RESIDUAL;
      execstore_type_to_enum["JACOBIAN"] = EXEC_JACOBIAN;
      execstore_type_to_enum["TIMESTEP"] = EXEC_TIMESTEP;
    }
  }


  template<>
  TimeSteppingScheme stringToEnum(const std::string & s)
  {
    initTimeSteppingMap();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!timesteppingscheme_type_to_enum.count(upper))
      mooseError("Unknown time stepping scheme");

    return timesteppingscheme_type_to_enum[upper];
  }

  template<>
  ExecFlagType stringToEnum(const std::string & s)
  {
    initExecStoreType();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!execstore_type_to_enum.count(upper))
      mooseError("Unknown execution flag");

    return execstore_type_to_enum[upper];
  }

}
