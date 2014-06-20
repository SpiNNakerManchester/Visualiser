__author__ = 'stokesa6'


class AbstractPage(object):

    def __init__(self, windows, main_pages, real_pages):
        """constructor for an abstract page

        :param windows: the list of currently opened windows (for the container\
                        to keep track of tis open windows)
        :param main_pages: the container for currently opened pages(for the \
                        container to keep track of tis open windows)
        :param real_pages: another container
        :type windows: iterable
        :type main_pages: iterable
        :type real_pages: iterable
        :return: should never be called
        :rtype: None
        :raise None: this object does not raise any known exceptions
        """
        self.windows = windows
        self.main_pages = main_pages
        self.real_pages = real_pages
        self.page = None
        self.real_pages.append(self)



    def recieved_spike(self, details):
        """the emthod used to process a spike for a page

        :param details: the details of a spike
        :type details: string
        :return None (should never be called directly)
        :rtype: None
        :raise NotImplementedError: as should not be called directly
        """
        raise NotImplementedError

    def reset_values(self):
        """the method used to reset any data objects

        :return None (should never be called directly)
        :rtype: None
        :raise NotImplementedError: as should not be called directly
        """
        raise NotImplementedError

    def is_page(self):
        """helper method that states if the page is a real page or a window

        :return: True or False
        :rtype: bool
        :raise NotImplementedError: as should not be called directly
        """
        raise NotImplementedError
