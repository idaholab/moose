#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <chrono>

#include "braceexpr.h"
#include "parse.h"

class Flags;
std::vector<std::string> parseOpts(int argc, char ** argv, Flags & flags);

int findParam(int argc, char ** argv);
int validate(int argc, char ** argv);
int format(int argc, char ** argv);
int merge(int argc, char ** argv);
int diff(int argc, char ** argv);
int common(int argc, char ** argv);
int subtract(int argc, char ** argv);
int profile(int argc, char ** argv);

bool
globCompare(const std::string & candidate,
            const std::string & pattern,
            std::size_t c = 0,
            std::size_t p = 0)
{
  if (p == pattern.size())
    return c == candidate.size();

  // '*' matches an arbitrary number of characters
  if (pattern[p] == '*')
  {
    for (; c < candidate.size(); ++c)
      if (globCompare(candidate, pattern, c, p + 1))
        return true;
    return globCompare(candidate, pattern, c, p + 1);
  }

  // '?' matches exactly one character
  if (pattern[p] != '?' && pattern[p] != candidate[c])
    return false;

  return globCompare(candidate, pattern, c + 1, p + 1);
}

std::string
readInput(const std::string & fname)
{
  if (fname == "-")
    return std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
  else
  {
    std::ifstream f(fname);
    if (!f)
      throw std::runtime_error("Can't open '" + fname + "'");
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  }
}

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "must specify a subcommand\n";
    return 1;
  }

  std::string subcmd(argv[1]);

  if (subcmd == "find")
    return findParam(argc - 2, argv + 2);
  else if (subcmd == "validate")
    return validate(argc - 2, argv + 2);
  else if (subcmd == "format")
    return format(argc - 2, argv + 2);
  else if (subcmd == "merge")
    return merge(argc - 2, argv + 2);
  else if (subcmd == "diff")
    return diff(argc - 2, argv + 2);
  else if (subcmd == "common")
    return common(argc - 2, argv + 2);
  else if (subcmd == "subtract")
    return subtract(argc - 2, argv + 2);
  else if (subcmd == "profile")
    return profile(argc - 2, argv + 2);
  else if (subcmd == "braceexpr")
  {
    std::stringstream ss;
    for (std::string line; std::getline(std::cin, line);)
      ss << line << std::endl;

    hit::BraceExpander expander;
    hit::EnvEvaler env;
    hit::RawEvaler raw;
    expander.registerEvaler("env", env);
    expander.registerEvaler("raw", raw);
    std::cout << expander.expand(nullptr, ss.str()) << "\n";

    return 0;
  }

  std::cerr << "unrecognized subcommand '" << subcmd << "'\n";
  return 1;
}

struct Flag
{
  bool arg;
  bool vec;
  bool have;
  std::string val;
  std::vector<std::string> vec_val;
  std::string help;
};

class Flags
{
public:
  Flags(const std::string & usage) : usage_msg(usage) {}
  void add(std::string name, std::string help, std::string def = "__NONE__")
  {
    if (def == "__NONE__")
      flags[name] = {false, false, false, "", {}, help};
    else
      flags[name] = {true, false, false, def, {}, help};
  }

  void addVector(std::string name, std::string help)
  {
    flags[name] = {false, true, false, "", {}, help};
  }

  bool have(std::string flag) { return flags.count(flag) > 0 && flags[flag].have; }
  std::string val(std::string flag) { return flags[flag].val; }
  std::vector<std::string> vecVal(std::string flag) { return flags[flag].vec_val; }
  std::string usage()
  {
    std::stringstream ss;
    ss << usage_msg << "\n";
    for (auto & pair : flags)
    {
      auto flag = pair.second;
      if (flag.arg)
        ss << "-" << pair.first << " <arg>    " << flag.help << " (default='" << flag.val << "')\n";
      else
      {
        if (flag.vec)
          ss << "-" << pair.first << "    " << flag.help << '\n';
        else
          ss << "-" << pair.first << "    " << flag.help << " (default=false)\n";
      }
    }
    return ss.str();
  }

  std::map<std::string, Flag> flags;
  std::string usage_msg;
};

std::vector<std::string>
parseOpts(int argc, char ** argv, Flags & flags)
{
  int i = 0;
  for (; i < argc; i++)
  {
    std::string arg = argv[i];
    // -- marks the start of positional arguments
    if (arg == "--")
    {
      i++;
      break;
    }

    if (arg[0] != '-')
      break;
    else if (arg.length() == 1)
      return std::vector<std::string>(1, "-");

    std::string flagname = arg.substr(1);
    if (flagname[0] == '-')
      flagname = flagname.substr(1);

    if (flags.flags.count(flagname) == 0)
      throw std::runtime_error("unknown flag '" + arg + "'");

    auto & flag = flags.flags[flagname];
    flag.have = true;
    if (flag.arg)
    {
      i++;
      flag.val = argv[i];
    }
    else if (flag.vec)
      while (i < argc - 1 && argv[i + 1][0] != '-')
      {
        i++;
        flag.vec_val.push_back(argv[i]);
      }
  }

  std::vector<std::string> positional;
  for (; i < argc; i++)
    positional.push_back(argv[i]);
  return positional;
}

class DupParamWalker : public hit::Walker
{
public:
  DupParamWalker() {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, hit::Node * n) override
  {
    std::string prefix = n->type() == hit::NodeType::Field ? "parameter" : "section";

    if (_have.count(fullpath) > 0)
    {
      auto existing = _have[fullpath];
      if (_duplicates.count(fullpath) == 0)
      {
        errors.push_back(
            hit::errormsg(existing, prefix, " '", fullpath, "' supplied multiple times"));
        _duplicates.insert(fullpath);
      }
      errors.push_back(hit::errormsg(n, prefix, " '", fullpath, "' supplied multiple times"));
    }
    _have[n->fullpath()] = n;
  }

  std::vector<std::string> errors;

private:
  std::set<std::string> _duplicates;
  std::map<std::string, hit::Node *> _have;
};

struct Pattern
{
  std::string param, value;
  bool negate;
};

Pattern
splitParamValue(const std::string & pv)
{
  auto equal_sign = pv.find('=');
  auto not_equal_sign = pv.find("!=");

  if (equal_sign == std::string::npos && not_equal_sign == std::string::npos)
    return Pattern{pv, "*", false};

  if (not_equal_sign != std::string::npos)
    return Pattern{std::string(pv, 0, not_equal_sign),
                   std::string(pv, not_equal_sign + 2, std::string::npos),
                   true};
  return Pattern{
      std::string(pv, 0, equal_sign), std::string(pv, equal_sign + 1, std::string::npos), false};
}

int
findParam(int argc, char ** argv)
{
  Flags flags(
      "hit find [flags] <parameter-pattern> <file>...\n  Specify '-' as a file name to accept "
      "input from stdin.\n  A pattern has the form param[=value] and wildcards (*,?) may be used");
  flags.add("f", "only show file name");
  flags.add("i", "case insensitive matches");
  flags.add("v", "show only files that don't match");
  flags.addVector(
      "p",
      "additional parameter patterns that must match the parent of the matched pattern (e.g. "
      "-p type=Diffusion variable=u). The -p option must be followed by another option or "
      "terminated with a double dash '--' followed by the list of filenames.");
  flags.add("h", "print help");
  flags.add("help", "print help");
  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("p") && positional.empty())
  {
    std::cerr << "Add a -- after the values provided to the -p option.";
    return 1;
  }

  if (flags.have("h") || flags.have("help") || positional.size() < 2)
  {
    std::cerr << flags.usage();
    return 1;
  }

  const bool file_only = flags.have("f");
  const bool case_insensitive = flags.have("i");
  const bool invert = flags.have("v");

  // parse main search pattern
  auto pattern = splitParamValue(case_insensitive ? hit::lower(positional[0]) : positional[0]);

  // get additional parameters
  auto pargs = flags.vecVal("p");
  std::vector<Pattern> additional_parameters;
  for (auto & p : pargs)
    additional_parameters.push_back(splitParamValue(case_insensitive ? hit::lower(p) : p));

  std::size_t n_matches = 0;
  std::size_t n_misses = 0;

  for (std::size_t i = 1; i < positional.size(); i++)
  {
    auto n_matches_old = n_matches;

    // load and parse input
    std::string fname(positional[i]);
    std::string input = readInput(fname);
    std::unique_ptr<hit::Node> root;
    try
    {
      root.reset(hit::parse(fname, input));
    }
    catch (std::exception & err)
    {
      std::cerr << err.what() << "\n";
      continue;
    }

    // gather parameters
    hit::GatherParamWalker::ParamMap params;
    hit::GatherParamWalker walker(params);
    root->walk(&walker);

    // search parameters
    for (const auto & p : params)
      if (globCompare(case_insensitive ? hit::lower(p.first) : p.first, pattern.param))
      {
        auto n = p.second;
        auto v = n->strVal();

        // if a value was given, make sure it matches, too
        if (globCompare(case_insensitive ? hit::lower(v) : v, pattern.value) == pattern.negate)
          continue;

        // if aditional -p arguments were given, check those, too
        bool ap_match = true;
        for (const auto & ap : additional_parameters)
        {
          // first check if the parent exists
          auto parent = n->parent();
          if (parent)
          {
            // look for the parameter name in the parent (and if it exists and a value was given
            // also match that)
            auto other_match = parent->find(ap.param);
            if (!other_match || globCompare(case_insensitive ? hit::lower(other_match->strVal())
                                                             : other_match->strVal(),
                                            ap.value) == ap.negate)
            {
              ap_match = false;
              break;
            }
          }
          else if (!ap.negate)
          {
            // no parent exists, so this cannot be a match, unless this is a != pattern
            ap_match = false;
            break;
          }
        }
        if (!ap_match)
          break;

        n_matches++;

        if (!invert)
        {
          if (file_only)
            std::cout << n->filename() << "\n";
          else
            std::cout << n->filename() << ":" << n->line() << ' ' << p.first << " = " << v << "\n";
        }
      }

    if (invert && n_matches_old == n_matches)
    {
      std::cout << positional[i] << "\n";
      n_misses++;
    }
  }

  auto num = invert ? n_misses : n_matches;
  if (!file_only)
    std::cout << num << (num == 1 ? " match" : " matches") << " found.\n";
  return num == 0;
}

// the style file is of the format:
//
//     [format]
//         indent_string = "  "
//         line_length = 100
//         canonical_section_markers = true
//
//         [sorting]
//             [pattern]
//                 section = "[^/]+/[^/]+"
//                 order = "type"
//             []
//             [pattern]
//                 section = ""
//                 order = "Mesh ** Executioner Outputs"
//             []
//         []
//     []
//
// where all fields are optional and the sorting section is also optional.  If the sorting section
// is present, you can have as many patterns as you want, but each pattern section must have
// 'section' and 'order' fields.
int
format(int argc, char ** argv)
{
  Flags flags(
      "hit format [flags] <file>...\n  Specify '-' as a file name to accept input from stdin.");
  flags.add("h", "print help");
  flags.add("help", "print help");
  flags.add("i", "modify file(s) inplace");
  flags.add("style", "hit style file detailing format to use", "");

  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help"))
  {
    std::cout << flags.usage();
    return 0;
  }

  if (positional.size() < 1)
  {
    std::cout << flags.usage();
    return 1;
  }

  hit::Formatter fmt;

  if (flags.have("style"))
  {
    try
    {
      std::string fname(flags.val("style"));
      std::ifstream f(fname);
      std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
      fmt = hit::Formatter(flags.val("style"), input);
    }
    catch (std::exception & err)
    {
      std::cerr << "invalid format style " << err.what() << "\n";
      return 1;
    }
  }

  int ret = 0;
  for (int i = 0; i < positional.size(); i++)
  {
    const std::string fname(positional[i]);
    const std::string input = readInput(fname);
    try
    {
      auto fmted = fmt.format(fname, input);
      if (flags.have("i") && fname != "-")
      {
        std::ofstream output(fname);
        output << fmted << "\n";
      }
      else
        std::cout << fmted;
    }
    catch (std::exception & err)
    {
      std::cerr << err.what() << "\n";
      ret = 1;
      continue;
    }
  }

  return ret;
}

std::unique_ptr<hit::Node>
readMerged(const std::vector<std::string> & input_filenames)
{
  std::unique_ptr<hit::Node> combined_root;

  for (auto & input_filename : input_filenames)
  {
    std::string input = readInput(input_filename);
    std::unique_ptr<hit::Node> root(hit::parse(input_filename, input));
    hit::explode(root.get());

    if (!combined_root)
      combined_root = std::move(root);
    else
      hit::merge(root.get(), combined_root.get());
  }

  return combined_root;
}

int
merge(int argc, char ** argv)
{
  Flags flags("hit merge [flags] -output outfile <file>...\n  Specify '-' as a file name to accept "
              "input from stdin.");
  flags.add("h", "print help");
  flags.add("help", "print help");
  flags.add("output", "Output file", "");

  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help"))
  {
    std::cout << flags.usage();
    return 0;
  }

  if (positional.size() < 1 || !flags.have("output"))
  {
    std::cout << flags.usage();
    return 1;
  }

  hit::Node * root = nullptr;
  for (int i = 0; i < positional.size(); i++)
  {
    const std::string fname(positional[i]);
    const std::string input = readInput(fname);
    if (root)
      hit::merge(hit::parse(fname, input), root);
    else
      root = hit::parse(fname, input);
  }

  std::string fname(flags.val("output"));
  if (fname == "-")
    std::cout << root->render();
  else
  {
    std::ofstream output(fname);
    output << root->render();
  }

  return 0;
}

int
diff(int argc, char ** argv)
{
  Flags flags("hit diff left.i right.i\nhit diff -left <files> -right <files>\n  Compare (merged) "
              "inputs on the left with "
              "(merged) inputs on the right.\n");
  flags.add("v", "verbose diff");
  flags.add("C", "output color");
  flags.add("color", "output color");
  flags.add("common", "show common parts on both sides");
  flags.add("h", "print help");
  flags.add("help", "print help");
  flags.addVector("left", "Left hand inputs");
  flags.addVector("right", "Right hand inputs");

  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help"))
  {
    std::cout << flags.usage();
    return 0;
  }

  if (flags.have("left") != flags.have("right") || (flags.have("left") && positional.size() > 0) ||
      (!flags.have("left") && positional.size() != 2))
  {
    std::cout << flags.usage();
    return 1;
  }

  bool use_color = flags.have("C") || flags.have("color");

  // terminal colors
  std::string color_red = use_color ? "\33[31m" : "";
  std::string color_green = use_color ? "\33[32m" : "";
  std::string color_blue = use_color ? "\33[34m" : "";
  std::string color_yellow = use_color ? "\33[33m" : "";
  std::string color_default = use_color ? "\33[39m" : "";

  auto left_files =
      flags.have("left") ? flags.vecVal("left") : std::vector<std::string>{positional[0]};
  auto right_files =
      flags.have("right") ? flags.vecVal("right") : std::vector<std::string>{positional[1]};
  auto left = readMerged(left_files);
  auto right = readMerged(right_files);

  if (!left || !right)
    return 1;

  std::cout << "Left hand side:\n";
  for (const auto & file : left_files)
    std::cout << "    " << file << '\n';

  std::cout << "\nRight hand side:\n";
  for (const auto & file : right_files)
    std::cout << "    " << file << '\n';

  std::cout << '\n';

  hit::GatherParamWalker::ParamMap left_params;
  hit::GatherParamWalker::ParamMap right_params;

  hit::GatherParamWalker left_walker(left_params);
  hit::GatherParamWalker right_walker(right_params);

  left->walk(&left_walker, hit::NodeType::Field);
  right->walk(&right_walker, hit::NodeType::Field);

  // verbose outputs
  std::stringstream diff_val;
  std::stringstream missing_left;
  std::stringstream missing_right;
  hit::Section missing_left_root("");
  hit::Section missing_right_root("");
  hit::Section common_root("");

  // params on left but not on right
  for (const auto & lparam : left_params)
  {
    auto it = right_params.find(lparam.first);
    if (it == right_params.end())
    {
      missing_right << color_red << lparam.first << color_blue << " (" << lparam.second->filename()
                    << ':' << lparam.second->line() << ")" << color_default
                    << " is missing on the right.\n";
      missing_right_root.addChild(lparam.second->clone(/*absolute_path = */ true));
    }
    else
    {
      if (lparam.second->strVal() != it->second->strVal())
        diff_val << "    " << color_yellow << lparam.first << color_blue << " ("
                 << lparam.second->filename() << ':' << lparam.second->line() << ")"
                 << color_default << " has differing values\n      '" << color_red
                 << lparam.second->strVal() << color_default << "' ->"
                 << (lparam.second->strVal().size() > 40 ? "\n      '" : " '") << color_green
                 << it->second->strVal() << color_default << "'\n";
      else
        common_root.addChild(lparam.second->clone(/*absolute_path = */ true));
    }
  }

  // params on right but not on left
  for (const auto & rparam : right_params)
    if (left_params.count(rparam.first) == 0)
    {
      missing_left << color_green << rparam.first << color_blue << " (" << rparam.second->filename()
                   << ':' << rparam.second->line() << ")" << color_default
                   << " is missing on the left.\n";
      missing_left_root.addChild(rparam.second->clone(/*absolute_path = */ true));
    }

  // output report
  bool verbose = flags.have("v");
  bool common = flags.have("common");

  if (common)
  {
    std::cout << "Common parameters:\n";
    hit::explode(&common_root);
    std::cout << common_root.render(4) << "\n\n";
    return 0;
  }
  else
  {
    if (missing_right.str().size())
    {
      std::cout << "Parameters removed left -> right:\n" << color_red;
      if (verbose)
        std::cout << missing_right.str() << '\n';
      else
      {
        hit::explode(&missing_right_root);
        std::cout << missing_right_root.render(4) << "\n\n";
      }
      std::cout << color_default;
    }

    if (missing_left.str().size())
    {
      std::cout << "Parameters added left -> right:\n" << color_green;
      if (verbose)
        std::cout << missing_left.str() << '\n';
      else
      {
        hit::explode(&missing_left_root);
        std::cout << missing_left_root.render(4) << "\n\n";
      }
      std::cout << color_default;
    }

    if (diff_val.str().size())
      std::cout << "Parameters with differing values:\n\n" << diff_val.str() << "\n\n";

    return (missing_left.str().size() + missing_right.str().size() + diff_val.str().size() > 0) ? 1
                                                                                                : 0;
  }
}

int
common(int argc, char ** argv)
{
  Flags flags("hit common <files>\n  Extract common parameters from all files.\n");
  flags.add("h", "print help");
  flags.add("help", "print help");
  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help") || positional.size() == 0)
  {
    std::cout << flags.usage();
    return positional.size() == 0 ? 1 : 0;
  }

  std::vector<std::unique_ptr<hit::Node>> roots;
  for (const auto & file : positional)
    roots.emplace_back(readMerged({file}));

  hit::GatherParamWalker::ParamMap common_params;
  hit::GatherParamWalker common_walker(common_params);
  roots[0]->walk(&common_walker);
  for (std::size_t i = 1; i < roots.size(); ++i)
  {
    hit::GatherParamWalker::ParamMap next_params;
    hit::GatherParamWalker next_walker(next_params);
    roots[i]->walk(&next_walker);

    for (auto it1 = common_params.begin(); it1 != common_params.end();)
    {
      auto it2 = next_params.find(it1->first);
      if (it2 == next_params.end() || it2->second->strVal() != it1->second->strVal())
        it1 = common_params.erase(it1);
      else
        ++it1;
    }
  }

  hit::Section common_root("");
  for (const auto & param : common_params)
    common_root.addChild(param.second->clone(/*absolute_path = */ true));
  hit::explode(&common_root);
  std::cout << common_root.render() << '\n';

  return 0;
}

int
subtract(int argc, char ** argv)
{
  Flags flags(
      "hit subtract base.i remove.i\n  Build the input base.i minus remove.i by removing all "
      "parameters listed in remove.i from base.i.\n");
  flags.add("h", "print help");
  flags.add("help", "print help");
  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help") || positional.size() != 2)
  {
    std::cout << flags.usage();
    return positional.size() != 2 ? 1 : 0;
  }

  auto left = readMerged({positional[0]});
  auto right = readMerged({positional[1]});

  if (!left || !right)
    return 1;

  std::cerr << "Subtracting:\n    " << positional[1] << "\nfrom:\n    " << positional[0] << '\n';

  hit::GatherParamWalker::ParamMap right_params;
  hit::GatherParamWalker right_walker(right_params);
  hit::RemoveParamWalker left_walker(right_params);
  hit::RemoveEmptySectionWalker left_section_walker;

  right->walk(&right_walker);
  left->walk(&left_walker);
  left->walk(&left_section_walker);

  std::cout << left->render();

  return 0;
}

int
validate(int argc, char ** argv)
{
  if (argc < 1)
  {
    std::cerr << "please pass in an input file argument (or pass '-' to validate stdin).\n";
    return 1;
  }

  int ret = 0;
  for (int i = 0; i < argc; i++)
  {
    const std::string fname(argv[i]);
    const std::string input = readInput(fname);
    std::unique_ptr<hit::Node> root;
    try
    {
      root.reset(hit::parse(fname, input));
    }
    catch (std::exception & err)
    {
      std::cout << err.what() << "\n";
      ret = 1;
      continue;
    }

    DupParamWalker w;
    root->walk(&w, hit::NodeType::Field);
    for (auto & msg : w.errors)
      std::cout << msg << "\n";
  }
  return ret;
}

class Timer
{
public:
  Timer(const std::string & msg, int n) : _msg(msg), _n(n), _begin(std::chrono::steady_clock::now())
  {
  }
  ~Timer()
  {
    const auto end = std::chrono::steady_clock::now();
    std::cout << _msg << ": "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - _begin).count() / _n
              << " ms" << std::endl;
  }

private:
  const std::string _msg;
  const int _n;
  const std::chrono::steady_clock::time_point _begin;
};

int
profile(int argc, char ** argv)
{
  Flags flags("hit profile [flags] file\n  Specify '-' as a file name to accept "
              "input from stdin.");
  flags.add("h", "print help");
  flags.add("help", "print help");
  flags.add("n", "Number of parse cycles", "100");

  auto positional = parseOpts(argc, argv, flags);

  if (flags.have("h") || flags.have("help"))
  {
    std::cout << flags.usage();
    return 0;
  }

  if (positional.size() != 1)
  {
    std::cout << flags.usage();
    return 1;
  }

  std::string input;
  {
    Timer timer("Reading input", 1);
    input = readInput(positional[0]);
  }

  const unsigned int num_iteration = std::stoi(flags.val("n"));
  {
    Timer timer("Parsing input", num_iteration);
    for (unsigned int i = 0; i < num_iteration; ++i)
      volatile auto root = hit::parse(positional[0], input);
  }

  return 0;
}
