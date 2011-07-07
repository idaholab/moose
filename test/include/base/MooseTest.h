#ifndef MOOSETEST_H
#define MOOSETEST_H

//Forward Declaration
class Parser;

namespace MooseTest
{
void registerObjects();

void associateSyntax(Parser & p);
}

#endif /* MOOSETEST_H */
