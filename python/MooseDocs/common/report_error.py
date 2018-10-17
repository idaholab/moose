import mooseutils
from box import box
def report_error(message, page, info, traceback=None, prefix=u'ERROR'):
    """
    Helper for reporting error to logging module.

    Inputs:
        message[str]: The message to report, ERROR is automatically appended
        page[pages.Page]: The page that created the error
        info[Information]: The lexer information object
        traceback: The traceback (should be a string from traceback.format_exc())
    """
    title = '{}: {}'.format(prefix, message)
    filename = mooseutils.colorText('{}:{}\n'.format(page.source, info.line), 'RESET')
    src = mooseutils.colorText(box(info[0], line=info.line, width=100), 'LIGHT_CYAN')
    trace = u'\n' + mooseutils.colorText(traceback, 'GREY') if traceback else ''
    return u'\n{}\n{}{}{}\n'.format(title, filename, src, trace)
