<?php

// 
// Authentication handler.
// 
class Authentication
{

        private $config;
        private $handle;

        public function __construct($config)
        {
                $this->config = $config;
        }

        public function authenticated($user, $pass, $method)
        {
                error_log(sprintf("user: %s, pass: %s, method: %s", $user, $pass, $method));
                
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
                                        ldap_close($this->handle);
                                        return true;
                                } else {
                                        ldap_close($this->handle);
                                }
                        }
                }
                
                return false;
        }

}
