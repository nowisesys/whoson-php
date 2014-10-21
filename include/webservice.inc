<?php

// Copyright (C) 2012, 2014 Computing Department BMC, 
// Uppsala Biomedical Centre, Uppsala University.
// 
// File:   include/webservice.inc
// Author: Anders Lövgren
// Date:   2012-02-27

if (!defined("WEB_SERVICE_WSDL")) {
        define("WEB_SERVICE_WSDL", __DIR__ . "/webservice.wsdl");
}

require_once ('include/types.inc');
require_once ('include/query.inc');
require_once ('include/auth.inc');

/**
 * The web service handler class.
 * 
 * @property-read string $proto The SOAP service protocol (HTTP or HTTPS).
 * @property-read string $addr The SOAP service address.
 * @property-read string $host The SOAP service hostname.
 * @property-read int $port The SOAP service port (e.g. 80 or 443).
 * @property-read string $path The SOAP service path.
 * @property-read string $wsdl The SOAP service description (WSDL) URL.
 * @property-read string $file Server local location of WSDL document.
 * @property-read string $user The username for authentication.
 * @property-read string $pass The password for authentication.
 */
class WebService
{

        /**
         * The authentication configuration.
         * @var array 
         */
        private $auth;
        /**
         * The database connection parameters.
         * @var array 
         */
        private $data;
        /**
         * Error report options.
         * @var array 
         */
        private $error = array(
                'send' => false,
                'log'  => true
        );

        /**
         * Constructor.
         * @param array $config Configuration options.
         */
        public function __construct($config)
        {
                $this->auth = new Authentication($config['auth']);
                $this->data = $config['db'];
                $this->error = $config['error'];
        }

        public function __get($name)
        {
                switch ($name) {
                        case "proto":
                                return (isset($_SERVER['HTTPS']) ? "https" : "http");
                        case "addr":
                                return $_SERVER['SERVER_ADDR'];
                        case "host":
                                return $_SERVER['SERVER_NAME'];
                        case "port":
                                return $_SERVER['SERVER_PORT'];
                        case "path":            // Script path
                                return $_SERVER['SCRIPT_NAME'];
                        case "wsdl":            // WSDL URL
                                return sprintf("%s://%s:%d/%s", $this->proto, $this->host, $this->port, $this->path);
                        case "file":            // WSDL filename
                                return WEB_SERVICE_WSDL;
                        case "user":
                                return isset($_SERVER['PHP_AUTH_USER']) ? $_SERVER['PHP_AUTH_USER'] : "";
                        case "pass":
                                return isset($_SERVER['PHP_AUTH_PW']) ? $_SERVER['PHP_AUTH_PW'] : "";
                }
        }

        /**
         * Setup this object as a SOAP handler.
         */
        public function handle()
        {
                $soap = new SoapServer($this->file);         // Using WSDL mode
                $soap->setObject($this);

                try {
		        // foreach(getallheaders() as $name => $value) {
			//         error_log(__METHOD__ . ': ' . $name . ' -> ' . $value);
			// }
		        // error_log(__METHOD__ . ": " . file_get_contents("php://input"));
		        
                        $soap->handle();
                } catch (\Exception $exception) {
                        if ($this->error['log']) {
                                error_log($exception);
                        }
                        if ($this->error['send']) {
                                $soap->fault("Receiver", $exception->getMessage());
                        }
                }
        }

        /**
         * Check authorization.
         * 
         * Check if access is granted to method using current credentials. If not
         * authorized, simply call exit() to terminate script execution.
         * 
         * @param string $method The called SOAP method.
         */
        private function authorize($method)
        {
                if (!$this->auth->authenticated($this->user, $this->pass, $method)) {
                        header('WWW-Authenticate: Basic realm="WhosOn"');
                        header('HTTP/1.0 401 Unauthorized');
                        exit(1);
                }
        }

        /**
         * Get database connection.
         * @return \PDO
         */
        private function getConnection()
        {
                if (isset($this->data['options'])) {
                        $pdo = new PDO($this->data['dsn'], $this->data['username'], $this->data['password'], $this->data['options']);
                        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
                        return $pdo;
                } else {
                        $pdo = new PDO($this->data['dsn'], $this->data['username'], $this->data['password']);
                        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
                        return $pdo;
                }
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

                $this->authorize(__METHOD__);

                $obj->addr = filter_input(INPUT_SERVER, 'REMOTE_ADDR');
                $obj->host = gethostbyaddr($obj->addr);
                $obj->time = strftime('%Y-%m-%d %H:%M:%S');

                $sql = "INSERT INTO logons(username, domain, workstation, hwaddress, hostname, ipaddress, starttime)"
                    . " VALUES(:username,:domain,:workstation,:hwaddress,:hostname,:ipaddress,:starttime)";

                $dbh = $this->getConnection();
                $sth = $dbh->prepare($sql);

                $sth->execute(array(
                        ':username'    => $obj->user,
                        ':domain'      => $obj->domain,
                        ':workstation' => $obj->computer,
                        ':hwaddress'   => $obj->hwaddr,
                        ':hostname'    => $obj->host,
                        ':ipaddress'   => $obj->addr,
                        ':starttime'   => $obj->time
                    )
                );

                return new CreateLogonEventResponse($dbh->lastInsertId());
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

                $this->authorize(__METHOD__);

                $obj->time = strftime('%Y-%m-%d %H:%M:%S');

                $sql = sprintf("UPDATE logons SET endtime = :endtime WHERE id = :id AND endtime IS NULL");

                $dbh = $this->getConnection();
                $sth = $dbh->prepare($sql);

                $sth->bindParam(':endtime', $obj->time, PDO::PARAM_STR);
                $sth->bindParam(':id', $obj->eventID, PDO::PARAM_INT);

                $sth->execute();

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
                // error_log("(" . __METHOD__ . ") " . print_r($obj, true));

                $this->authorize(__METHOD__);

                $res = array(); // Array of LogonEvent objects

                $qbd = new QueryBuilder($obj->filter, $obj->match);
                $qbd->build();

                $dbh = $this->getConnection();
                $rec = $dbh->query($qbd->getSelect());

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

                $this->authorize(__METHOD__);

                $sql = "SELECT * FROM logons WHERE username = :username AND domain = :domain AND workstation = :workstation AND endtime IS NULL";

                $dbh = $this->getConnection();
                $sth = $dbh->prepare($sql);

                $sth->execute(array(
                        ':username'    => $obj->user,
                        ':domain'      => $obj->domain,
                        ':workstation' => $obj->computer
                ));

                $res = new QueryResult($sth);
                $res->read();
                
                return new FindLogonEventResponse($res->next());
        }

        /**
         * Send service description (WSDL).
         */
        public function sendDescription()
        {
	        header("Content-type: application/wsdl+xml");
                echo str_replace("@location@", $this->wsdl, file_get_contents($this->file));
        }

}

?>
