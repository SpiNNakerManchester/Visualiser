__author__ = 'stokesa6'

import gtk
import logging
from visualiser import exceptions
from visualiser.abstract_page import AbstractPage
logger = logging.getLogger(__name__)


class VisualiserMain(object):
    """main entrace to the gtk thread"""

    def __init__(self, parent):
        """constructor for the main gtk thread.

        :param parent: the main vis thread
        :type parent: visualiser_thread
        :return a new vis page container
        :rtype: visulaiser.visualiser_page_container
        :raise None: this method does not raise any known exception

        """
        #define the window
        self._pages = None
        self._main_menu = None
        self._vbox = None
        self._accel = None
        #holder for the different pages (hopefully for speed of represnetation)
        self._open_windows = list()
        #start making the main window
        self._window = self._set_up_main_window()
        self._create_the_notebook_that_holds_pages()
        self._add_menus()
        #set current page to X
        self._pages.set_current_page(2)
        #display window
        self._window.show()
        self._parent = parent

    #initlise main window
    def _set_up_main_window(self):
        """creates the main window to which pages and notebooks are attached

        :return the main window
        :rtype: gtk.Window
        :raise None:  this method does not raise any known exception
        """
        window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        window.connect("delete_event", self.delete_event)
        window.connect("destroy", self.destroy)
        window.set_border_width(10)
        window.set_default_size(500, 500)
        window.set_title("SpinnView")
        self._vbox = gtk.VBox(False, 2)
        self._vbox.show()
        window.add(self._vbox)
        self._accel = gtk.AccelGroup()
        window.add_accel_group(self._accel)
        return window


    def _create_the_notebook_that_holds_pages(self):
        """creates the container for the future pages

        :return: None
        :rtype: None
        :raise None:  this method does not raise any known exception
        """
        self._pages = gtk.Notebook()
        self._pages.show()
        self._pages.set_tab_pos(gtk.POS_TOP)
        self._vbox.pack_start(self._pages, True, True, 1)
        self._pages.show_tabs = True
        self._pages.show_border = True

    def _add_menus(self):
        """ adds basic menus for the vis
        :return: None
        :rtype: None
        :raise None:  this method does not raise any known exception
        """
        self._main_menu = gtk.MenuBar()
        self._vbox.pack_start(self._main_menu, False, False, 0)
        #add file menuitem
        file_menu_main = gtk.MenuItem("File")
        self._main_menu.append(file_menu_main)

        #add file submenu
        file_sub_menu = gtk.Menu()
        file_menu_main.set_submenu(file_sub_menu)
        #add file sub menu items
        new_item = gtk.ImageMenuItem(gtk.STOCK_NEW, self._accel)
        key, mod = gtk.accelerator_parse("<Control>N")
        new_item.add_accelerator("activate", self._accel,
                                 key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(new_item)

        load_item = gtk.ImageMenuItem(gtk.STOCK_HARDDISK, self._accel)
        load_item.set_label("Load")
        key, mod = gtk.accelerator_parse("<Control>O")
        load_item.add_accelerator("activate", self._accel,
                                  key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(load_item)

        sep = gtk.SeparatorMenuItem()
        file_sub_menu.append(sep)

        exit_item = gtk.ImageMenuItem(gtk.STOCK_QUIT, self._accel)
        key, mod = gtk.accelerator_parse("<Control>Q")
        exit_item.add_accelerator("activate", self._accel,
                                  key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(exit_item)
        exit_item.connect("activate", self.destroy)

        self._main_menu.show_all()
        self._main_menu.show()

    def add_menu_item(self, label, function_call):
        """helper method to allow front ends to add function buttons to the \
           main menu

        :param label: the label to use on the menu item (end user visable)
        :param function_call: the method to call when the button is activated
        :type label: str
        :type function_call: method
        :return: None
        :rtype: None
        :raise visualiser.exceptions.VisualiserInvalidInputException: when the \
               label or function_call are not valid params. Such as not a str\
               or a function call.
        """
        if isinstance(label, str) and callable(function_call):
            menu_item = gtk.MenuItem(label)
            self._main_menu.append(menu_item)
            menu_item.connect("activate", function_call)

            self._main_menu.show_all()
            self._main_menu.show()
        else:
            if not isinstance(label, str):
                raise exceptions.\
                    VisualiserInvalidInputException("the label is not a string")
            if not callable(function_call):
                raise exceptions.\
                    VisualiserInvalidInputException("param functioncall is not"
                                                    "a callable object")

    def add_page(self, page, page_label):
        """helper method to allow the front ends to add a page to the visulaiser

        :param page: the page to be used by the vis
        :param page_label: the label presented at the top of the page (tab)
        :type page: dirivitive from visualiser.abstract_page to be added to the\
                    vis
        :rtype: dirivitive from visualiser.abstract_page
        :raise visualiser.exceptions.VisualiserInvalidInputException: when the \
               label or page are not valid params. Such as not a str\
               or a dirivitive of abstract page.
        """
        if isinstance(page_label, str) and issubclass(page, AbstractPage):
            if page.is_page():
                self._pages.append_page(page, page_label)
            else:
                self._open_windows.append(page)
        else:
            if not isinstance(page_label, str):
                raise exceptions.\
                    VisualiserInvalidInputException("the label is not a string")
            if not issubclass(page, AbstractPage):
                raise exceptions.\
                    VisualiserInvalidInputException("the page is not a "
                                                    "recogonised page")

    def delete_event(self, widget, event, data=None):
        """method to kill a widgit
        :param widget: the widget that called delete
        :param event: the event used
        :param data: optimal param for things
        :type: widget: a gtk object
        :type event: a gtk event
        :type data: ???
        :return: False
        :rtype: bool
        :raise None:  this method does not raise any known exception
        """
        return False

    def destroy(self, widget, data=None):
        """method to kill the gui
        :param widget: the widget that called delete
        :param data: optimal param for things
        :type: widget: a gtk object
        :type data: ???
        :return: False
        :rtype: bool
        :raise None:  this method does not raise any known exception
        """
        for qui in self._open_windows:
            qui.destroy()
        self._window.destroy()
        gtk.main_quit()
        self._parent.stop()

    def main(self):
        """default main method for the gtk loop

        :return: None
        :rtype: None
        :raise None:  this method does not raise any known exception
        """
        gtk.main()

#test boot method
if __name__ == "__main__":
    hello = VisualiserMain(None)
    hello.main()