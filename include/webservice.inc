<?php

// Copyright (C) 2012 Computing Department BMC, 
// Uppsala Biomedical Centre, Uppsala University.
// 
// File:   include/webservice.inc
// Author: Anders Lövgren
// Date:   2012-02-27

if (!defined("WEB_SERVICE_WSDL")) {
        define("WEB_SERVICE_WSDL", __DIR__ . "/webservice.wsdl");
}

require_once ('MDB2.php');

require_once ('conf/database.conf');
require_once ('include/types.inc');
require_once ('include/query.inc');

class WebService
{

        // 
        // Virtual members ($this->name)
        // 
        public function __get($name)
        {
                switch ($name) {
                        case "proto":
                                return (isset($_SERVER['HTTPS']) ? "https" : "http");
                        case "addr":
                                return $_SERVER['SERVER_ADDR'];
                        case "port":
                                return $_SERVER['SERVER_PORT'];
                        case "path":            // Script path
                                return $_SERVER['SCRIPT_NAME'];
                        case "wsdl":            // WSDL URL
                                return sprintf("%s://%s:%d/%s", $this->proto, $this->addr, $this->port, $this->path);
                        case "file":            // WSDL filename
                                return WEB_SERVICE_WSDL;
                }
        }

        // 
        // Setup this object as a SOAP handler.
        // 
        public function handle()
        {
                $soap = new SoapServer($this->file);         // Using WSDL mode
                $soap->setObject($this);
                $soap->handle();
        }

        /**
         * Web service API (callback method).
         * 
         * @param CreateLogonEvent $obj
         * @return CreateLogonEventResponse
         */
        public function createLogonEvent($obj)
        {
                // error_log("(" . __METHOD__ . ") " . print_r($obj, true));

                $obj->addr = $_SERVER['REMOTE_ADDR'];
                $obj->host = gethostbyaddr($obj->addr);
                $obj->time = strftime('%Y-%m-%d %H:%M:%S');

                $sql = sprintf("INSERT INTO logons(username, domain, workstation, hwaddress, hostname, ipaddress, starttime) VALUES('%s','%s','%s','%s','%s','%s','%s')", $obj->user, $obj->domain, $obj->computer, $obj->hwaddr, $obj->host, $obj->addr, $obj->time);

                $dbo = MDB2::singleton($GLOBALS['dsn']);
                if (PEAR::isError($dbo)) {
                        trigger_error($dbo->getMessage(), E_USER_ERROR);
                }

                $res = $dbo->query($sql);
                if (PEAR::isError($res)) {
                        trigger_error($res->getMessage(), E_USER_ERROR);
                }

                $eid = $dbo->lastInsertId();
                if (PEAR::isError($eid)) {
                        trigger_error($eid->getMessage(), E_USER_ERROR);
                }

                return new CreateLogonEventResponse($eid);
        }

        /**
         * Web service API (callback method).
         * 
         * @param CloseLogonEvent $obj 
         * @return CloseLogonEventResponse
         */
        public function closeLogonEvent($obj)
        {
                // error_log("(" . __METHOD__ . ") " . print_r($obj, true));

                $obj->time = strftime('%Y-%m-%d %H:%M:%S');

                $dbo = MDB2::singleton($GLOBALS['dsn']);
                if (PEAR::isError($dbo)) {
                        trigger_error($dbo->getMessage(), E_USER_ERROR);
                }

                $sql = sprintf("UPDATE logons SET endtime = '%s' WHERE id = %d", $obj->time, $obj->eventID);
                $res = $dbo->query($sql);
                if (PEAR::isError($res)) {
                        trigger_error($res->getMessage(), E_USER_ERROR);
                }

                return new CloseLogonEventResponse();
        }

        /**
         * Web service API (callback method).
         * 
         * @param FindLogonEvents $obj 
         * @return FindLogonEventsResponse
         */
        public function findLogonEvents($obj)
        {
                error_log("(" . __METHOD__ . ") " . print_r($obj, true));

                $res = array (); // Array of LogonEvent objects
                
                $dbo = MDB2::singleton($GLOBALS['dsn']);
                if (PEAR::isError($dbo)) {
                        trigger_error($dbo->getMessage(), E_USER_ERROR);
                }
                $dbo->setFetchMode(MDB2_FETCHMODE_OBJECT);
                
                $qbd = new QueryBuilder($obj->filter, $obj->match);
                $qbd->build();
                
                $rec = $dbo->query($qbd->getSelect());
                
                $qrs = new QueryResult($rec);
                $qrs->fill($res);

                return new FindLogonEventsResponse(new ArrayOfLogonEvent($res));
        }

        /**
         * Web service API (callback method).
         * 
         * @param FindLogonEvent $obj 
         * @return FindLogonEventResponse
         */
        public function findLogonEvent($obj)
        {
                // error_log("(" . __METHOD__ . ") " . print_r($obj, true));

                $sql = sprintf("SELECT * FROM logons WHERE username = '%s' AND domain = '%s' AND workstation = '%s' AND endtime IS NULL", $obj->user, $obj->domain, $obj->computer);

                $dbo = MDB2::singleton($GLOBALS['dsn']);
                if (PEAR::isError($dbo)) {
                        trigger_error($dbo->getMessage(), E_USER_ERROR);
                }
                $dbo->setFetchMode(MDB2_FETCHMODE_OBJECT);

                $res = $dbo->query($sql);
                if (PEAR::isError($res)) {
                        trigger_error($res->getMessage(), E_USER_ERROR);
                }

                $qry = new QueryResult($res);
                $rec = $qry->next();

                return new FindLogonEventResponse($rec);
        }

        // 
        // Send service description (WSDL).
        // 
        public function sendDescription()
        {
                echo str_replace("@location@", $this->wsdl, file_get_contents($this->file));
        }

}

?>
