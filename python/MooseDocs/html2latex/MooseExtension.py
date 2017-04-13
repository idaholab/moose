from Extension import Extension
from moose_elements import *

class MooseExtension(Extension):
    """
    Aggregates the MOOSE specific element objects into an extension for html to latex conversion.
    """
    def extend(self, translator):

        config = self.getConfigs()

        translator.elements.add('moose_internal_links', moose_internal_links(), '<a')
        translator.elements.add('moose_external_links', moose_markdown_links(site=config['site']), '<a')
        translator.elements.add('moose_inline_code', moose_inline_code(), '<code')

        translator.elements.add('moose_bib', moose_bib(), '<ol')
        translator.elements.add('moose_bib_span', moose_bib_span(), '<moose_internal_links')
        translator.elements.add('moose_slider', moose_slider(), '_begin')
        translator.elements.add('moose_buildstatus', moose_buildstatus(), '_begin')
        translator.elements.add('admonition_div', admonition_div(), '<div')
        translator.elements.add('moose_code_div', moose_code_div(), '_begin')
        translator.elements.add('moose_pre_code', moose_pre_code(), '<pre_code')
        translator.elements.add('moose_pre', moose_pre(), '<pre')
        translator.elements.add('moose_table', moose_table(), '<table')
        translator.elements.add('moose_img', moose_img(), '<img')
        translator.elements.add('moose_diagram', moose_diagram(), '<moose_img')

        if not config['hrule']:
            translator.elements.add('moose_hide_hr', moose_hide_hr(), '<hr')
