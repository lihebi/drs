Distributed Random Servers with Timed Labels for Synchronization over Named Data Network


== Overview ==

The project is an implementation for the paper: home.ustc.edu.cn/~lihebi/pdf/drstl.pdf

== FILeS ==
scenarios/drs.cc: the main loop file for DRSTL
         /chrono.cc: the main loop file for ChronoSync
extensions/ : application codes, including drs, chrono, and common functions
          /hebi.cc: common functions
          /pugixml.cc: pugixml library, for XML parse
          /userreader.cc: functions for reading user configuration files
          /drs : main drs app source codes
          /drs/drs-app.cc : main app class and its implementation
              /drs-record.cc : record class
              /drs-record-container.cc : record container class
          /chrono : main chrono app source codes
          /chrono/chrono-app.cc : main app class and its implementation
                 /chrono-digest-tree.cc : digest tree class
                 /chrono-digest-log.cc : digest log class
                 /chrono-message.cc : message class
topo/ : topology configuration files
    /generator.py : script for generating topo file
    /*.txt : topo files

== INSTALL ==
* install the ndn customized NS3 platform
* install the ndnsim wrapper for NS3
* configure the environment variables to make the env required available

== USAGE ==:

* compile the project use the code:
  ./waf

* a better way to run the project is using the bash script:
  ./drs.sh
  or
  ./chrono.sh

* a list of environment variables should be set, you can use the script:
  . env.sh

* data can be analysised using the python scripts:
  ./data_drs.py
  ./data_chrono.py
