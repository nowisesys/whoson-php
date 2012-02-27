<?php

// Copyright (C) 2012 Computing Department BMC, 
// Uppsala Biomedical Centre, Uppsala University.
// 
// File:   include/query.inc
// Author: Anders Lövgren
// Date:   2012-02-27

if (!defined("DATETIME_IS_NULL")) {
        define("DATETIME_IS_NULL", '0000-00-00 00:00:00');
}

// 
// Define values matching LogonEventMatch enum (C#/ASP.NET)
// 
if (!defined('LOGON_EVENT_MATCH_BEFORE')) {
        define('LOGON_EVENT_MATCH_BEFORE', 'Before');
}
if (!defined('LOGON_EVENT_MATCH_BETWEEN')) {
        define('LOGON_EVENT_MATCH_BETWEEN', 'Between');
}
if (!defined('LOGON_EVENT_MATCH_AFTER')) {
        define('LOGON_EVENT_MATCH_AFTER', 'After');
}
if (!defined('LOGON_EVENT_MATCH_EXACT')) {
        define('LOGON_EVENT_MATCH_EXACT', 'Exact');
}
if (!defined('LOGON_EVENT_MATCH_ACTIVE')) {
        define('LOGON_EVENT_MATCH_ACTIVE', 'Active');
}
if (!defined('LOGON_EVENT_MATCH_CLOSED')) {
        define('LOGON_EVENT_MATCH_CLOSED', 'Closed');
}

/**
 * Class supporting query building from filter and match options.
 *
 * @author Anders Lövgren (QNET/BMC CompDept)
 */
class QueryBuilder
{

        /**
         * @var LogonEvent
         */
        private $filter;

        /**
         * @var string 
         */
        private $match;

        /**
         * @param LogonEvent $filter 
         * @param string $match
         */
        public function __construct($filter, $match)
        {
                $this->filter = $filter;
                $this->match = $match;
        }

        /**
         * Set the query filter.
         * @param LogonEvent $filter 
         */
        public function setFilter($filter)
        {
                $this->filter = $filter;
        }

        /**
         * Set the match filter.
         * @param string $match
         */
        public function setMatch($match)
        {
                $this->match = $match;
        }

        /**
         * Build the query string (SQL).
         * @return string 
         */
        public function build()
        {
                $this->where = array ();
                $this->query = "SELECT * FROM logons";

                switch ($this->match) {
                        case LOGON_EVENT_MATCH_EXACT:
                                $this->filterExact();
                                break;
                        case LOGON_EVENT_MATCH_BEFORE:
                                $this->filterBefore();
                                break;
                        case LOGON_EVENT_MATCH_AFTER:
                                $this->filterAfter();
                                break;
                        case LOGON_EVENT_MATCH_BETWEEN:
                                $this->filterBetween();
                                break;
                        case LOGON_EVENT_MATCH_ACTIVE:
                                $this->filterActive();
                                break;
                        case LOGON_EVENT_MATCH_CLOSED:
                                $this->filterClosed();
                                break;
                }

                if (count($this->where) != 0) {
                        $this->query .= " WHERE ";
                        for ($i = 0; $i < count($this->where); ++$i) {
                                if ($i > 0) {
                                        $this->query .= " AND ";
                                }
                                $this->query .= $this->where[$i];
                        }
                }
                $this->query .= " ORDER BY id";
                return $this->query;
        }

        /**
         * Get the SQL SELECT string from last build() call.
         * @return string 
         */
        public function getSelect()
        {
                return $this->query;
        }

        private static function enabled($var)
        {
                if (is_null($var)) {
                        return false;
                } elseif (is_string($var)) {
                        if (($time = strtotime($var)) !== false) {
                                return $time > 0;
                        } else {
                                return strlen($var) != 0;
                        }
                } elseif (is_numeric($var)) {
                        return $var != 0;
                }
        }

        private function filterActive()
        {
                $this->where[] = "endtime IS NULL";
        }

        private function filterClosed()
        {
                $this->where[] = "endtime IS NOT NULL";
        }

        private function filterBetween()
        {
                if (self::enabled($this->filter->StartTime) && self::enabled($this->filter->EndTime)) {
                        $this->where[] = sprintf("starttime BETWEEN '%s' AND '%s'", $this->filter->StartTime, $this->filter->EndTime);
                }
        }

        private function filterBefore()
        {
                if ($this->filter->EventID != 0) {
                        $this->where[] = sprintf("id < %s", $this->filter->EventID);
                }
                if (self::enabled($this->filter->StartTime)) {
                        $this->where[] = sprintf("starttime < '%s'", $this->filter->StartTime);
                }
                if (self::enabled($this->filter->EndTime)) {
                        $this->where[] = sprintf("endtime < '%s'", $this->filter->EndTime);
                }
        }

        private function filterAfter()
        {
                if ($this->filter->EventID != 0) {
                        $this->where[] = sprintf("id > %d", $this->filter->EventID);
                }
                if (self::enabled($this->filter->StartTime)) {
                        $this->where[] = sprintf("starttime > '%s'", $this->filter->StartTime);
                }
                if (self::enabled($this->filter->EndTime)) {
                        $this->where[] = sprintf("endtime > '%s'", $this->filter->EndTime);
                }
        }

        private function filterExact()
        {
                if ($this->filter->EventID != 0) {
                        $this->where[] = sprintf("id = %d", $this->filter->EventID);
                }
                if (self::enabled($this->filter->Username)) {
                        $this->where[] = sprintf("username = '%s'", $this->filter->Username);
                }
                if (self::enabled($this->filter->Domain)) {
                        $this->where[] = sprintf("domain = '%s'", $this->filter->Domain);
                }
                if (self::enabled($this->filter->HwAddress)) {
                        $this->where[] = sprintf("hwaddress = '%s'", $this->filter->HwAddress);
                }
                if (self::enabled($this->filter->IpAddress)) {
                        $this->where[] = sprintf("ipaddress = '%s'", $this->filter->IpAddress);
                }
                if (self::enabled($this->filter->Hostname)) {
                        $this->where[] = sprintf("hostname = '%s'", $this->filter->Hostname);
                }
                if (self::enabled($this->filter->Workstation)) {
                        $this->where[] = sprintf("workstation = '%s'", $this->filter->Workstation);
                }
                if (self::enabled($this->filter->StartTime)) {
                        $this->where[] = sprintf("starttime = '%s'", $this->filter->StartTime);
                }
                if (self::enabled($this->filter->EndTime)) {
                        $this->where[] = sprintf("endtime = '%s'", $this->filter->EndTime);
                }
        }

}

/**
 * Class supporting query marshalling of query result.
 * 
 * The $res argument to the constructor should be an valid handle returned by
 * a successful call to MDB2::query(). There are two ways to use this class
 * for fetching data:
 * 
 * 1. Implicit fetch data:
 * 
 *    $res = array();
 *    $obj = new QueryResult($qrs);
 *    $obj->fill($res);
 * 
 * 2. Explicit fetch data:
 * 
 *    $res = array();
 *    $obj = new QueryResult($qrs);
 *    while ($obj->read()) {                    // fetch data while true
 *        array_push($res, $obj->next());       // create object from last read
 *    }
 * 
 * The obvious reason to have two different ways of fetching data is to support
 * filtering using predicates callback objects and SPL.
 * 
 * @author Anders Lövgren (QNET/BMC CompDept)
 */
class QueryResult
{

        private $res;
        private $data;

        public function __construct(&$res)
        {
                $this->res = $res;
        }

        public function __destruct()
        {
                $this->res->free();
        }

        /**
         * Fill array with all data.
         * 
         * @param array $list The output data
         * @return array The input array
         */
        public function & fill(&$list)
        {
               while ($this->read()) {
                        $list[] = $this->next();
                }
                return $list;
        }

        /**
         * Fetch next record and store in internal variable. Returns true
         * if successful and false if last fetch returned null (EOF).
         * @return bool 
         */
        public function read()
        {
                $this->data = $this->res->fetchRow();
                return $this->data != null;
        }

        /**
         * Create LogonEvent object from last fetched record. Returns null if
         * internal data buffer is null.
         * 
         * @return null|\LogonEvent 
         */
        public function next()
        {
                if(!isset($this->data)) {
                        return null;
                }

                if (!isset($this->data->id)) {
                        $this->data->id = 0;
                }
                if (!isset($this->data->username)) {
                        $this->data->username = null;
                }
                if (!isset($this->data->domain)) {
                        $this->data->domain = null;
                }
                if (!isset($this->data->hwaddress)) {
                        $this->data->hwaddress = null;
                }
                if (!isset($this->data->ipaddress)) {
                        $this->data->ipaddress = null;
                }
                if (!isset($this->data->hostname)) {
                        $this->data->hostname = null;
                }
                if (!isset($this->data->workstation)) {
                        $this->data->workstation = null;
                }
                if (!isset($this->data->starttime)) {
                        $this->data->starttime = DATETIME_IS_NULL;
                }
                if (!isset($this->data->endtime)) {
                        $this->data->endtime = DATETIME_IS_NULL;
                }

                return new LogonEvent(
                        $this->data->id,
                        $this->data->username,
                        $this->data->domain,
                        $this->data->hwaddress,
                        $this->data->ipaddress,
                        $this->data->hostname,
                        $this->data->workstation,
                        strtotime($this->data->starttime),
                        strtotime($this->data->endtime));
        }

}

?>
