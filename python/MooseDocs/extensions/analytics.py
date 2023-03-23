#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
from ..base import Extension, HTMLRenderer

LOG = logging.getLogger('MooseDocs.AnalyticsExtension')

def make_extension(**kwargs):
    return AnalyticsExtension(**kwargs)

class AnalyticsExtension(Extension):
    """
    Adds the ability to capture page visits via Google Analytics.
    """
    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        config['google_measurement_id'] = (None, 'The Google Analytics measurement ID')
        return config

    def extend(self, reader, renderer):
      """
      Adds the google tag scripts as needed if a Google Analytics
      measurement ID is provided.
      """
      if isinstance(renderer, HTMLRenderer):
          mid = self.get('google_measurement_id')
          if mid:
              renderer.addJavaScript('gtag', f'https://www.googletagmanager.com/gtag/js?id={mid}', head=True)

              tag_contents = "window.dataLayer = window.dataLayer || [];"
              tag_contents += "function gtag(){dataLayer.push(arguments);}"
              tag_contents += "gtag('js', new Date());"
              tag_contents += f"gtag('config', '{mid}');"
              renderer.addJavaScript('google_analytics', tag_contents, head=True)
