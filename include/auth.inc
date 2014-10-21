<?php

// Copyright (C) 2012, 2014 Computing Department BMC, 
// Uppsala Biomedical Centre, Uppsala University.
// 
// File:   include/auth.inc
// Author: Anders Lövgren
// Date:   2012-02-27

/**
 * Authentication handler class.
 */
class Authentication
{

        /**
         * Configuration options.
         * @var array 
         */
        private $config;
        /**
         * LDAP conection handle.
         * @var resource 
         */
        private $handle;

        /**
         * Constructor
         * @param array $config Configuration options.
         */
        public function __construct($config)
        {
                $this->config = $config;
        }

        /**
         * Destructor.
         */
        public function __destruct()
        {
                if (isset($this->handle)) {
                        ldap_close($this->handle);
                }
        }

        /**
         * Authenticate to access method (if required) using supplied credentials.
         * 
         * Returns true if successful authenticated or if method don't require
         * authentication.
         * 
         * @param string $user The username.
         * @param string $pass The password.
         * @param string $method The requested SOAP service method.
         * @return boolean 
         * @throws RuntimeException
         */
        public function authenticated($user, $pass, $method)
        {
                // error_log(__METHOD__ . ': ' . sprintf("user: %s, pass: %s, method: %s", $user, $pass, $method));

                if (!in_array($method, $this->config['methods'])) {
                        error_log(print_r($this->config, true));
                        return true;
                }

                if (!empty($user) && !empty($pass)) {
                        if (isset($this->config['simple'])) {
                                if (($this->config['simple']['user'] == $user) &&
                                    ($this->config['simple']['pass'] == $pass)) {
                                        return true;
                                }
                        }
                        if (isset($this->config['ldap'])) {
                                if (!($this->handle = ldap_connect($this->config['ldap']['host']))) {
                                        throw new RuntimeException(sprintf("Failed connect to %s", $this->config['ldap']['host']));
                                }
                                if (ldap_bind($this->handle, $user, $pass)) {
                                        return true;
                                }
                        }
                }

                return false;
        }

}
