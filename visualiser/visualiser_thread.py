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
    def __init__(self, timeout=0.0):
        """constructor for the vis thread

        :param timeout: param for the thread
        :type timeout: int
        :return: None
        :rtype: None
        :raise None: does not raise any known exception

        """
        gtk.threads_init()
        self.visulaiser_main = None
        self.visulaiser_listener = None
        threading.Thread.__init__(self)
        self.bufsize = 65536
        self.done = False
        self.port = None
        self.finish_event = threading.Event()
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

    def set_bufsize(self, bufsize):
        """supports changing of the bufsize

        :param bufsize: the associated new bufsize
        :type bufsize: int
        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self.bufsize = bufsize

    def stop(self):
        """stops the visualiser thread
        :return: None
        :rtype: None
        :raise None:   does not raise any known exceptions
        """
        logger.info("[visualiser] Stopping")
        self.done = True
        if self.has_board and self.visulaiser_listener is not None:
            self.visulaiser_listener.stop()

    def run(self):
        """opening method for this thread

        :return: None
        :rtype: None
        :raise None:  does not raise any known exceptions
        """
        self.visulaiser_main = VisualiserMain(self)
        self.visulaiser_listener.set_visualiser(self.visulaiser_main)

        logger.info("[visualiser] Starting")
        if self.visulaiser_listener is not None:
            self.visulaiser_listener.start()

        gtk.threads_enter()
        self.visulaiser_main.main()
        gtk.threads_leave()
        logger.debug("[visualiser] Exiting")


if __name__ == "__main__":
    logging.basicConfig()
    logger.setLevel(logging.DEBUG)
    visulaiser = VisualiserThread(None, 1, 1, True)
    visulaiser.start()
