from .exceptions import MooseDocsException


def boolean(setting: str) -> bool:
    """
    Checks if the provided setting can be parsed as a bool.
    Parameters of type bool are allowed to be the text 'true' or 'false'
    the check is also case insensitive.
    If the parse is sucessful a bool will be returned if not a
    MooseDocsException will be raised

    Parameters
    ----------
    setting : str
        The string containing the user supplied value for the setting

    Returns
    -------
    out : bool

    Raises
    ------
    MooseDocsException :
        if the value is not either 'True' of 'False', case insensitive
    """

    if setting.lower() == 'true':
        return True
    if setting.lower() == 'false':
        return False
    raise MooseDocsException(f"Unable to parse provided input '{
                             setting}' as a bool.")
