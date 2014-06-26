__author__ = 'stokesa6'
import gtk


class AbstractPage(gtk.Bin):

    def __init__(self):
        """constructor for an abstract page
        :return: should never be called
        :rtype: None
        :raise None: this object does not raise any known exceptions
        """
        pass

    def is_page(self):
        """helper method that states if the page is a real page or a window

        :return: True or False
        :rtype: bool
        :raise NotImplementedError: as should not be called directly
        """
        raise NotImplementedError
