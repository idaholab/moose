#ifndef THMSYNTAX_H
#define THMSYNTAX_H

class Syntax;
class ActionFactory;

namespace THM
{
void associateSyntax(Syntax & syntax);
void registerActions(Syntax & syntax);
}

#endif // THMSYNTAX_H
