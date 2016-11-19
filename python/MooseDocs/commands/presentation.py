import os
import shutil
import jinja2

def presentation_options(parser, subparser):
  """
  Command line options for the slide generator.
  """
  reveal_default = os.path.join(os.getenv('HOME'), 'projects', 'reveal.js')

  presentation = subparser.add_parser('presentation', help="Convert a markdown file to an html presentation.")
  presentation.add_argument('input', type=str, help="The markdown file to convert to slides.")
  presentation.add_argument('--reveal', '-r', default=reveal_default, help="The name of the reveal.js location where this presentation is to be installed (default: %(default)s).")
  presentation.add_argument('--output', '-o', default='index.html', help="The default html file to create within the revel directory (default: %(default)s).")
  presentation.add_argument('--template', type=str, default='basic.html', help="The template html file to utilize (default: %(default)s).")

  presentation.add_argument('--title', type=str, default="MOOSE Presentation", help="The title of the document.")

  return presentation

def presentation(reveal=None, input=None, output=None, template=None, **kwargs):
  """
  MOOSE markdown presentation blaster.
  """

  # Check that the reveal.js exists and contains reveal.js
  if not os.path.exists(reveal):
    raise Exception("The reveal.js directory {} does not exist.".format(options.reveal))
  elif not os.path.exists(os.path.join(options.reveal, 'js', 'reveal.js')):
    raise Exception("The reveal.js directory {} does not seem to be a clone of the reveal.js repository.".format(options.reveal))

  # Check that filename exists
  if not os.path.exists(input):
    raise Exception("The file {} does not exists.".format(input))

  # Copy moose.css
  src = os.path.join(os.path.dirname(__file__), 'css', 'moose.css')
  dst = os.path.join(options.reveal, 'css', 'moose.css')
  shutil.copyfile(src, dst)

  # Read the markdown and convert to html
  with open(options.input, 'r') as fid:
    md = fid.read()

  parser = markdown.Markdown(extensions=[MooseDocs.extensions.MooseMarkdown(slides=True), 'markdown_include.include', 'smarty', 'admonition', 'mdx_math', 'toc', 'extra'])
  html = parser.convert(md)


  # Apply changes to template
  env = jinja2.Environment(loader=jinja2.FileSystemLoader('templates'))
  template = env.get_template(template)

  # Inject the code and write output
  complete = template.render(content=html, **kwargs)
  with open(os.path.join(reveal, output), 'w') as fid:
    fid.write(complete)
