
// libMesh includes
#include "libmesh.h"
#include "point.h"
#include "parameters.h"
#include "vector_value.h"
#include "equation_systems.h"
#include "explicit_system.h"

namespace Moose
{
  Number initial_value (const Point& p,
                        const Parameters& parameters,
                        const std::string& sys_name,
                        const std::string& var_name)
  {
    if(parameters.have_parameter<Real>("initial_"+var_name))
      return parameters.get<Real>("initial_"+var_name);
    
    return 0;
  }

  Gradient initial_gradient (const Point& p,
                             const Parameters& parameters,
                             const std::string& sys_name,
                             const std::string& var_name)
  {
    return RealGradient();
  }

  void initial_condition(EquationSystems& es, const std::string& system_name)
  {
    ExplicitSystem & system = es.get_system<ExplicitSystem>(system_name);
    
    system.project_solution(initial_value, initial_gradient, es.parameters);
  }
}
