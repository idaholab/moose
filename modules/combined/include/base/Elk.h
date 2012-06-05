#ifndef ELK_H
#define ELK_H

#include "Syntax.h"

namespace Elk
{
  /**
   * Registers the Kernels, BCs, and Materials provided in Elk.
   */
  void registerObjects();

  void associateSyntax(Syntax & syntax);
}

#endif //ELK_H
