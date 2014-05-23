<?php

// Copyright (C) 2012 Computing Department BMC, 
// Uppsala Biomedical Centre, Uppsala University.
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
require_once('include/webservice.inc');

if (isset($_REQUEST['wsdl'])) {
        $service = new WebService($GLOBALS['auth']);
        $service->sendDescription();
} else {
        $service = new WebService($GLOBALS['auth']);
        $service->handle();
}
?>
