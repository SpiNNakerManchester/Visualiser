__author__ = 'stokesa6'

import threading
import thread
import time
from visualiser.visualiser_page_container import VisualiserMain
from port_listener import PortListener

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
    def __init__(self, machine_time_step, time_scale_factor, has_board,
                 timeout=0.0, start_simulation_method = None):
        """

        """
        gtk.threads_init()
        self.machine_time_step = machine_time_step
        self.time_scale_factor = time_scale_factor
        self.has_board = has_board
        self.visulaiser_main = None
        self.visulaiser_listener = None
        threading.Thread.__init__(self)
        self.bufsize = 65536
        self.done = False
        self.port = None
        self.start_simulation_method = start_simulation_method
        self.finish_event = threading.Event()
        if timeout > 0:
            thread.start_new_thread(_timeout, (self, timeout))

    def start_now(self):
        """

        """
        self.start_simulation_method()
        self.finish_event.set()

    def wait_for_finish(self):
        """

        """
        self.finish_event.wait()

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
        :raise None:   does not raise any known exceptions
        """
        start_method = None
        print "Start: ", self.start_simulation_method
        if self.start_simulation_method is not None:
            start_method = getattr(self, "start_now")

        if self.has_board and self.port is not None:
            self.visulaiser_listener = PortListener(self.machine_time_step,
                                                    self.time_scale_factor)
            self.visulaiser_listener.set_port(self.port)
        else:
            logger.warn("you are running the visualiser without a board."
                        " Aspects of the visualiser may not work")
        self.visulaiser_main = VisualiserMain(self, start_method)
        self.visulaiser_listener.set_visualiser(self.visulaiser_main)

        logger.info("[visualiser] Starting")
        if self.visulaiser_listener is not None:
            self.visulaiser_listener.start()

        gtk.threads_enter()
        self.visulaiser_main.main()
        gtk.threads_leave()
        logger.debug("[visualiser] Exiting")

    def set_port(self, port):
        """sets the listeners port

        :return: None
        :rtype: None
        :raise None:   does not raise any known exceptions
        """
        self.port = port


if __name__ == "__main__":
    logging.basicConfig()
    logger.setLevel(logging.DEBUG)
    visulaiser = VisualiserThread(None, 1, 1, True)
    visulaiser.start()
