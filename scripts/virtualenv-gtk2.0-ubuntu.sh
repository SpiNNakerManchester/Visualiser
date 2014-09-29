#!/bin/bash
if [ -z "$VIRTUAL_ENV" ]; then
    echo "VIRTUAL_ENV environment variable has not been set.  Please make sure that you have activated a virtualenv before using this script"
    exit
fi

function create_link {
    if [ ! -e $VIRTUAL_ENV/lib/python2.7/site-packages/$1 ]; then
        ln -s /usr/lib/python2.7/dist-packages/$1 $VIRTUAL_ENV/lib/python2.7/site-packages/$1
    fi
}

create_link "cairo"
create_link "glib"
create_link "gobject"
create_link "gtk-2.0"
create_link "pygtk.pth"

echo "Links for GTK have been created"
