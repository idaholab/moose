
#include "braceexpr.h"

#include <cstdlib>

namespace hit
{

std::string
BraceNode::str(int indent)
{
  std::string indstr;
  for (int i = 0; i < indent; i++)
    indstr += "    ";

  if (_val != "")
    return indstr + _val + "\n";

  std::string s = indstr + "${\n";
  for (auto & n : _list)
  {
    s += n.str(indent + 1);
  }
  s += indstr + "}\n";
  return s;
}

BraceNode &
BraceNode::append()
{
  _list.push_back(BraceNode());
  return _list.back();
}

std::string
EnvEvaler::eval(std::list<std::string> & args) const
{
  std::string var = args.front();
  std::string val;
  auto data = std::getenv(var.c_str());
  if (data)
    val = data;
  return val;
}

std::string
RawEvaler::eval(std::list<std::string> & args) const
{
  std::string s;
  for (auto & arg : args)
    s += arg;
  return s;
}

size_t parseBraceNode(const std::string & input, size_t start, BraceNode & n);
size_t parseBraceBody(const std::string & input, size_t start, BraceNode & n);

void
BraceExpander::registerEvaler(const std::string & name, const Evaler & ev)
{
  _evalers[name] = &ev;
}

std::string
BraceExpander::expand(const std::string & input)
{
  std::string result = input;
  size_t start = 0;
  while ((start = result.find("${", start)) != std::string::npos)
  {
    BraceNode root;
    parseBraceNode(result, start, root);

    // skip brace expressions that are a single word - they are a special case for intra-input
    // replacement based on HIT paths.
    if (root.list().size() < 2)
    {
      start += root.len();
      continue;
    }

    auto replace_text = expand(root);
    result.replace(root.offset(), root.len(), replace_text);
    start = root.offset() + replace_text.size();
  }
  return result;
}

std::string
BraceExpander::expand(BraceNode & expr)
{
  auto args = expr.list();
  if (args.size() == 0)
    return expr.val();

  std::list<std::string> expanded_args;
  for (auto it = args.begin(); it != args.end(); ++it)
    expanded_args.push_back(expand(*it));

  auto cmd = expanded_args.front();
  if (_evalers.count(cmd) == 0)
    throw std::runtime_error("no valid evaler '" + cmd + "'");
  expanded_args.pop_front();
  return _evalers[cmd]->eval(expanded_args);
}

size_t
skipSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r";
  while (start < input.size() && space.find(input[start]) != std::string::npos)
    start++;
  return start;
}

size_t
untilSpace(const std::string & input, size_t start)
{
  const std::string space = "\n\t \r}";
  while (start < input.size() && space.find(input[start]) == std::string::npos)
    start++;
  return start;
}

size_t
parseBraceNode(const std::string & input, size_t start, BraceNode & n)
{
  n.offset() = start;
  start += 2; // eat opening "${"
  start = parseBraceBody(input, start, n);
  start = skipSpace(input, start);
  if (input[start] != '}')
    throw std::runtime_error("missing closing '}' in brace expression");
  start++; // eat closing "}"
  n.len() = start - n.offset();
  return start;
}

size_t
parseBraceBody(const std::string & input, size_t start, BraceNode & n)
{
  start = skipSpace(input, start);
  while (start < input.size() && input[start] != '}')
  {
    if (input.find("${", start) == start)
      start = parseBraceNode(input, start, n.append());
    else
    {
      auto end = untilSpace(input, start);
      auto & child = n.append();
      child.val() = input.substr(start, end - start);
      start = end;
    }
    start = skipSpace(input, start);
  }
  return start;
}

ExpandWalker::ExpandWalker(std::string fname, BraceExpander& expander)
  : _fname(fname), _expander(expander)
{
}

void
ExpandWalker::walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n)
{
  auto f = dynamic_cast<Field *>(n);
  if (!f)
    return;

  std::string s;
  try
  {
    s = _expander.expand(f->val());
  }
  catch (Error & err)
  {
    errors.push_back(errormsg(_fname, n, err.what()));
    return;
  }

  auto start = s.find("${");
  while (start < s.size())
  {
    auto end = s.find("}", start);
    if (end != std::string::npos)
    {
      auto var = s.substr(start + 2, end - (start + 2));

      auto curr = n;
      while ((curr = curr->parent()))
      {
        auto src = curr->find(var);
        if (src && src != n && src->type() == NodeType::Field)
        {
          used.push_back(pathJoin({curr->fullpath(), var}));
          s = s.substr(0, start) + curr->param<std::string>(var) +
              s.substr(end + 1, s.size() - (end + 1));

          if (end + 1 - start == f->val().size())
            f->setVal(s, dynamic_cast<Field *>(curr->find(var))->kind());
          else
            f->setVal(s);

          // move end back to the position of the end of the replacement text - not the replaced
          // text since the former is the one relevant to the string for remaining replacements.
          end = start + curr->param<std::string>(var).size();
          break;
        }
      }

      if (curr == nullptr)
        errors.push_back(
            errormsg(_fname, n, "no variable '", var, "' found for substitution expression"));
    }
    else
      errors.push_back(errormsg(_fname, n, "missing substitution expression terminator '}'"));
    start = s.find("${", end);
  }
  f->setVal(s);
}

} // namespace hit
