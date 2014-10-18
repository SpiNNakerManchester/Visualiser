import gtk
from abc import abstractmethod


class AbstractPage(object):

    def __init__(self, label):
        """constructor for an abstract _page
        :param label: the label of the _page
        :type label: gtk.Label
        :return: should never be called
        :rtype: None
        :raise None: this object does not raise any known exceptions
        """
        self._label = label

    def is_page(self):
        """helper method that states if the _page is a real _page or a window

        :return: True or False
        :rtype: bool
        :raise NotImplementedError: as should not be called directly
        """
        raise NotImplementedError

    @property
    def label(self):
        """helper method that returns the label of the _page

        :return:
        """
        return self._label

    @abstractmethod
    def get_frame(self):
        """
        method to force pages to return there frame.
        :return:
        """


