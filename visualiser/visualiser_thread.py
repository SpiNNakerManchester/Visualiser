__author__ = 'stokesa6'

import threading
import thread
import time
from visualiser.visualiser_page_container import VisualiserMain
import logging
logger = logging.getLogger(__name__)
import gtk


def _timeout(visualiser, timeout):
    time.sleep(timeout)
    visualiser.stop()


class VisualiserThread(threading.Thread):
    """

    """

    #sets up listeners
    def __init__(self, timeout=0.0, has_board=True):
        """constructor for the vis thread

        :param timeout: param for the thread
        :type timeout: int
        :return: None
        :rtype: None
        :raise None: does not raise any known exception

        """
        gtk.threads_init()
        self._has_board = has_board
        self._visulaiser_main = VisualiserMain(self)
        self._visulaiser_listener = None
        threading.Thread.__init__(self)
        self._bufsize = 65536
        self._done = False
        self._port = None
        self._finish_event = threading.Event()
        self.setDaemon(True)
        if timeout > 0:
            thread.start_new_thread(_timeout, (self, timeout))


    def set_timeout(self, timeout):
        """supports changing how long to timeout

        :param timeout: the associated length of time for a timeout
        :type timeout: int
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        print("Timeout set to %f" % timeout)
        if timeout > 0:
            thread.start_new_thread(_timeout, (self, timeout))

    def set_visualiser_listener(self, listener):
        self._visulaiser_listener = listener

    def set_bufsize(self, bufsize):
        """supports changing of the bufsize

        :param bufsize: the associated new bufsize
        :type bufsize: int
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self._bufsize = bufsize

    def stop(self):
        """stops the visualiser thread
        :return: None
        :rtype: None
        :raise None:   does not raise any known exceptions
        """
        logger.info("[visualiser] Stopping")
        self._done = True
        if self._has_board and self._visulaiser_listener is not None:
            self._visulaiser_listener.stop()

    def run(self):
        """opening method for this thread

        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        gtk.threads_enter()
        self._visulaiser_main.main()
        gtk.threads_leave()
        logger.debug("[visualiser] Exiting")

    def add_page(self, page, label):
        """helper method to allow front ends to add pages to the main container

        :param page: the page to add to the container
        :param label: the label used by the contianer to mark the page
        :type page: a derived from abstract page
        :type label: str
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self._visulaiser_main.add_page(page, label)

    def add_menu_item(self, label, function_call):
        """helper method to allow front ends to add menu items to the main
        container

        :param function_call: the callback for when the menu item is clicked
        :param label: the label used by the contianer to mark the menu item
        :type function_call: a callable object
        :type label: str
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self._visulaiser_main.add_menu_item(self, label, function_call)

    def remove_menu_item(self, label):
        """helper method to allow front ends to remove menu items to the main
           container
        :param label: the label used by the contianer to mark the menu item
        :type label: str
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self._visulaiser_main.remove_menu_item(self, label)

    def remove_page(self, page):
        """helper method to allow front ends to remove pages to the main
           container

        :param page: the page to add to the container
        :type page: a derived from abstract page
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self._visulaiser_main.remove_page(page)

    def does_page_exist(self, page):
        """helper method to check if a page already exists
        :param page: the page to locate in the container
        :type page: a derived from abstract page
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        return self._visulaiser_main.does_page_exist(page)

    def pages(self):
        """helper method that returns the collection of pages

        :return: list of pages
        :rtype: list
        :raise None:  does not raise any known exceptions
        """
        return self._visulaiser_main.pages