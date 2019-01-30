<?php

// 
// Copyright (C) 2012-2018 Anders Lövgren, BMC-IT, Uppsala University.
// Copyright (C) 2018-2019 Anders Lövgren, Nowise Systems.
// 
// File:   source/index.php
// Author: Anders Lövgren
// Date:   2012-02-27
// 
// The public page exposing the web service. All processing is handled by the
// WebService class. 
// 
// This page simply detects if WSDL mode is requested, otherwise it lets the
// service class handle the SOAP request.
// 

require_once ('conf/config.inc');
require_once ('conf/database.conf');

require_once ('include/webservice.inc');

if (filter_has_var(INPUT_GET, 'wsdl')) {
        $service = new WebService($config);
        $service->sendDescription();
} else {
        $service = new WebService($config);
        $service->handle();
}
