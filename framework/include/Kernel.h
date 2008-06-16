#include "system.h"

/** 
 * The Kernel class is responsible for calculating the residuals for various
 * physics.
 * 
 */
class Kernel
{
public:
  Kernel(){};

  /** 
   * This constructor should be used most often.  It initializes all internal
   * references needed for residual computation.
   * 
   * @param system The system this variable is in
   * @param var_name The variable this Kernel is going to compute a residual for.
   */
  Kernel(System * system, std::string var_name);

  virtual ~Kernel(){};

  computeResidual();
};

