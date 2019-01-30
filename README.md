# WhosOn logon accounting

WhosOn is a client/server system for logon accounting. Use either whoson-php 
or whoson-asp on server side, with whoson-sql for SQL support. Install the 
Linux, Mac OS X or Windows client on computers from who logon session statistics 
should be collected.

### Deployment:

Easiest is to record events by running the client as a logon/logoff script 
from i.e. an active directory GPO. The client can also be run as a service/daemon 
on client computers, monitoring user logons in the background.

### Records:

Recorded data has IP, hostname, MAC, username, domain and datetime as field
that is stored in the database. The client communicates with server side using
SOAP.

### Authentication:

By default, authentication is disabled on server side, but its possible to
enable HTTP basic authentication against i.e. LDAP or plain text file.

### This component:

Provides a server-side component of the WhosOn application. The whoson-php is 
functional equivalent to whoson-asp, but intended to be runned on Linux/UNIX 
hosts (under i.e. Apache).

### See also:

* [Web Service (PHP)](https://github.com/nowisesys/whoson-php)
* [Web Service (ASP.NET)](https://github.com/nowisesys/whoson-asp)
* [SQL Database (MySQL/MSSQL)](https://github.com/nowisesys/whoson-sql)
* [Client (Linux/Mac OS X)](https://github.com/nowisesys/whoson-linux)
* [Client (Windows)](https://github.com/nowisesys/whoson-windows)
