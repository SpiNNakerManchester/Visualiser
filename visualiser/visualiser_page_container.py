__author__ = 'stokesa6'

import gtk
import threading
import logging
logger = logging.getLogger(__name__)


class VisualiserMain(object):
    """

    """

    def __init__(self, parent, start_simulation_method=None):
        """

        """
        #define the window
        self.received = False
        self.pages = None
        self.real_pages = list()
        self.main_menu = None
        self.run_menu_item = None
        self.vbox = None
        self.accel = None
        #holder for the different pages (hopefully for speed of represnetation)
        self.open_windows = list()
        self.vertex_to_page_mapping = dict()
        self.start_simulation_method = start_simulation_method
        #start making the main window
        self.window = self._set_up_main_window()
        self._create_the_notebook_that_holds_pages()
        self.add_menus()
        #set current page to X
        self.pages.set_current_page(2)
        #display window
        self.window.show()
        self.parent = parent

    #initlise main window
    def _set_up_main_window(self):
        window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        window.connect("delete_event", self.delete_event)
        window.connect("destroy", self.destroy)
        window.set_border_width(10)
        window.set_default_size(500, 500)
        window.set_title("SpinnView")
        self.vbox = gtk.VBox(False, 2)
        self.vbox.show()
        window.add(self.vbox)
        self.accel = gtk.AccelGroup()
        window.add_accel_group(self.accel)
        return window


    def _create_the_notebook_that_holds_pages(self):
        '''
        sets up the original notebook for holding pages
        '''
        self.pages = gtk.Notebook()
        self.pages.show()
        self.pages.set_tab_pos(gtk.POS_TOP)
        self.vbox.pack_start(self.pages, True, True, 1)
        self.pages.show_tabs = True
        self.pages.show_border = True

    def add_menus(self):
        '''
        adds basic menus for the vis
        '''
        self.main_menu = gtk.MenuBar()
        self.vbox.pack_start(self.main_menu, False, False, 0)
        #add file menuitem
        file_menu_main = gtk.MenuItem("File")
        self.main_menu.append(file_menu_main)

        #add file submenu
        file_sub_menu = gtk.Menu()
        file_menu_main.set_submenu(file_sub_menu)
        #add file sub menu items
        new_item = gtk.ImageMenuItem(gtk.STOCK_NEW, self.accel)
        key, mod = gtk.accelerator_parse("<Control>N")
        new_item.add_accelerator("activate", self.accel,
                                 key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(new_item)

        load_item = gtk.ImageMenuItem(gtk.STOCK_HARDDISK, self.accel)
        load_item.set_label("Load")
        key, mod = gtk.accelerator_parse("<Control>O")
        load_item.add_accelerator("activate", self.accel,
                                  key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(load_item)

        sep = gtk.SeparatorMenuItem()
        file_sub_menu.append(sep)

        exit_item = gtk.ImageMenuItem(gtk.STOCK_QUIT, self.accel)
        key, mod = gtk.accelerator_parse("<Control>Q")
        exit_item.add_accelerator("activate", self.accel,
                                  key, mod, gtk.ACCEL_VISIBLE)
        file_sub_menu.append(exit_item)
        exit_item.connect("activate", self.destroy)

        if self.start_simulation_method is not None:
            self.run_menu_item = gtk.MenuItem("Run Now!")
            self.main_menu.append(self.run_menu_item)
            self.run_menu_item.connect("activate", self._run_item_selected)

        self.main_menu.show_all()
        self.main_menu.show()

    def _run_item_selected(self, widget):
        '''
        method to handle the clicking of the run now mode
        '''
        self.main_menu.remove(self.run_menu_item)
        start_thread = threading.Thread(target=self.start_simulation_method)
        start_thread.start()


    def delete_event(self, widget, event, data=None):
        '''
        method to kill the gui
        '''
        return False

    def destroy(self, widget, data=None):
        '''
        kill the gui
        '''
        for qui in self.open_windows:
            qui.destroy()
        self.window.destroy()
        gtk.main_quit()
        self.parent.stop()

    def main(self):
        '''
        default start method
        '''
        gtk.main()

#test boot method
if __name__ == "__main__":
    hello = VisualiserMain(None)
    hello.main()