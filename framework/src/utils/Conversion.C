#include "Conversion.h"
#include <map>

namespace Moose {

  std::map<std::string, TimeSteppingScheme> timesteppingscheme_type_to_enum;

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

}
