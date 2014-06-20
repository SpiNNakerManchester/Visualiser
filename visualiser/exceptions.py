__author__ = 'stokesa6'


class VisuliserException(Exception):
    """Superclass of all exceptions from the visuliser module.

    :raise None: does not raise any known exceptions"""
    pass


class VisualiserInvalidInputException(VisuliserException):
    """raised when a input param for a visualiser function is not valid.

    :raise None: does not raise any known exceptions"""
    pass