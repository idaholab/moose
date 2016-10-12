import os
import re
from markdown.preprocessors import Preprocessor
import logging
log = logging.getLogger(__name__)
import MooseDocs

class MooseMarkdownLinkPreprocessor(Preprocessor):
  """
  A preprocessor for creating automatic linking between markdown files.
  """

  def __init__(self, pages=None, **kwargs):
    super(MooseMarkdownLinkPreprocessor, self).__init__(**kwargs)
    self._pages = pages

  def run(self, lines):
    """
    The raw markdown lines are processed to find and create links between pages.
    """
    content = '\n'.join(lines)
    content = re.sub(r'(?<!`)\[(.*?)\]\((.*?\.md)(.*?)\)', self._linkSub, content)
    content = re.sub(r'(?<!`)\[(.*?\.md)(.*?)\]', self._bracketSub, content)
    return content.split('\n')

  def _bracketSub(self, match):
    """
    Substitution of bracket links: [Diffusion.md]

    Args:
      match[re.Match]: The python re.Match object.
    """
    name = self._findFile(match.group(1))
    if name:
      return '[{}](/{}{})'.format(name, name, match.group(2))
    return match.group(0)

  def _linkSub(self, match):
    """
    Substitution of links: [Test](Diffusion.md)

    Args:
      match[re.Match]: The python re.Match object.
    """
    name = self._findFile(match.group(2))
    if name:
      return '[{}](/{}{})'.format(match.group(1), name, match.group(3))
    return match.group(0)

  def _findFile(self, name):
    """
    Helper for getting complete path and warning if multiple items are found.

    Args:
      name[str]: Partial filename to locate.
    """
    # Create list of possible matches
    files = []
    for page in self._pages:
      if page.endswith(name):
        files.append(page)

    # Warning if multiple items located
    if len(files) > 1:
      msg = "Found multiple items listed with the name '{}':".format(name)
      for filename in files:
        msg += '\n    {}'.format(filename)
      log.warning(msg)
    if files:
      return files[0]
