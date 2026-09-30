from .exceptions import MooseDocsException


def boolean(setting: str) -> bool:
    """
    Checks if the provided setting can be parsed as a bool.
    Parameters of type bool are allowed to be the text 'true' or 'false'
    the check is also case insensitive.

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


def unsigned_integer(setting: str) -> int:
    """
    Checks if the provided setting is an integer and ensures that it is also positive

    Parameters
    ----------
    setting : str
        The string containing the user supplied value for the setting

    Returns
    -------
    out : an int in the range [0 \\infty]

    Raises
    ------
    MooseDocsException :
        if the value cannot be parsed as an int or if the value is negative
    """

    value = None

    try:
        value = int(setting)
    except Exception:
        raise MooseDocsException(f"Unable to parse input '{
                                 setting}' as an int.")

    if value < 0:
        raise MooseDocsException(
            f"Expected an unsigned int and a negative value '{value}' was provided")

    return value


def floating_point(setting: str) -> float:
    """
    Checks if the provided setting is a valid floating point value and ensures
    that it is also positive

    Parameters
    ----------
    setting : str
        The string containing the user supplied value for the setting

    Returns
    -------
    out : floating point value

    Raises
    ------
    MooseDocsException :
        if the value cannot be parsed as a float
    """

    try:
        return float(setting)
    except Exception:
        raise MooseDocsException(f"Unable to parse input '{
                                 setting}' as an int.")
