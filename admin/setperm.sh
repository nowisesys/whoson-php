#!/bin/sh
#
# Fix permission on database and configure files.
#
# Author: Anders Lövgren
# Date:   2012-02-27

confdir=conf

chmod 640 $confdir/database.conf
setfacl -m u:apache:r $confdir/database.conf
