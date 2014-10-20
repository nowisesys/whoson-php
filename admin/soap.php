<?php

// -------------------------------------------------------------------------------
//  Copyright (C) 2007-2012, 2014 Anders Lövgren
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// -------------------------------------------------------------------------------
// 
// This utility generates PHP classes matching the SOAP types defined
// by the WSDL read from a file or URL.
// 
//
// The script should only be run in CLI mode.
//
if (isset($_SERVER['SERVER_ADDR'])) {
        die("This script should be runned in CLI mode.\n");
}

ini_set("soap.wsdl_cache_enabled", "0"); // disabling WSDL cache 
// 
// This function generates SOAP type classes.
// 

function generate_classes($opts, $wsdl)
{
        $soap = new SoapClient($wsdl);
        if (!$soap) {
                die("(-) error: failed create soap client\n");
        }

        // print_r($soap->__getTypes());
        // var_dump($soap->__getTypes());

        foreach ($soap->__getTypes() as $type) {
                $lines = split("\n", $type);
                $class = "";
                $param = array ();
                foreach ($lines as $line) {
                        $parts = explode(" ", $line);
                        if ($parts[0] == "struct") {
                                $class = ucfirst($parts[1]);
                                if (preg_match("/^[a-z]*$/", $parts[1])) {
                                        $class .= "Params";
                                }
                        } elseif ($parts[0] == null) {
                                $name = rtrim($parts[2], ";");
                                $type = $parts[1];
                                $param[$name] = $type;
                        } elseif ($parts[0] == "}") {
                                // 
                                // Generate the class:
                                // 
                                printf("//\n");
                                printf("// Synopsis: %s(%s)\n", $class, implode(", ", array_values($param)));
                                printf("//\n");
                                printf("class %s {\n", $class);
                                foreach ($param as $name => $type) {
                                        if ($opts->phpdoc) {
                                                printf("    /**\n     * @var %s\n     */\n", $type);
                                                printf("    %s \$%s;\n", $opts->member, $name);
                                        } else {
                                                printf("    %s \$%s;\t// %s\n", $opts->member, $name, $type);
                                        }
                                        if ($opts->pretty) {
                                                printf("\n");
                                        }
                                }
                                if ($opts->phpdoc) {
                                        printf("    /**\n");
                                        foreach ($param as $name => $type) {
                                                printf("     * @param %s \$%s\n", $type, $name);
                                        }
                                        printf("     */\n");
                                }
                                if ($opts->constr) {
                                        $class = "__construct";
                                }
                                if (count($param)) {
                                        printf("    %s function %s($%s) {\n", $opts->access, $class, implode(", $", array_keys($param)));
                                } else {
                                        printf("    %s function %s() {\n", $opts->access, $class);
                                }
                                foreach ($param as $name => $type) {
                                        printf("        \$this->%s = \$%s;\n", $name, $name);
                                }
                                printf("    }\n");
                                printf("}\n\n");
                        } else if (!$opts->strict) {
                                printf("// ++++++++++++++++++++++++++++++++++++++++++\n");
                                printf("// Note: %s has type %s\n", $parts[1], $parts[0]);
                                printf("// ++++++++++++++++++++++++++++++++++++++++++\n\n");
                        } else {
                                die(sprintf("(-) error: unexpected part '%s'\n", $parts[0]));
                        }
                }
        }
}

function usage($prog)
{
        $wsdl = array (
            "remote" => "http://localhost/whoson?wsdl",
            "local" => "../include/webservice.wsdl.cache"
        );

        printf("%s - generates PHP classes from SOAP types\n", $prog);
        printf("\n");
        printf("Usage: %s [options...] <wsdl-location-url>\n", $prog);
        printf("Options:\n");
        printf("  --constr[=bool]:  Use __constructor declaration.\n");
        printf("  --access=str:     Visibility for constructor.\n");
        printf("  --member=str:     Member access (var, private, protected, ...\n");
        printf("  --phpdoc[=bool]:  Generate PHP documenator tags.\n");
        printf("  --pretty=[bool]:  Add some extra formatting.\n");
        printf("  --compact:        Same as --pretty=false.\n");
        printf("  --strict:         Halt on type errors.\n");
        printf("\n");
        printf("Note: The wsdl-location-url is either an local file or the URL\n");
        printf("to an WSDL location on an server.\n");
        printf("\n");
        foreach ($wsdl as $key => $url) {
                printf("Example (%s wsdl):\t%s %s\n", $key, $prog, $url);
        }
}

// 
// The main function.
// 
function main($argc, &$argv)
{
        $prog = basename(array_shift($argv));
        $opts = array (
            'constr' => true, // use __constructor()
            'access' => 'public', // visibility for constructor
            'member' => 'private', // member declaration
            'phpdoc' => false, // output PHPdoc tags
            'pretty' => true, // add some extra spacing
            'strict' => false // be more permissive
        );

        if ($argc == 1) {
                usage($prog);
                exit(1);
        }

        while (($opt = array_shift($argv))) {
                if (strchr($opt, '=')) {
                        list($key, $val) = explode('=', $opt);
                } else {
                        list($key, $val) = array ($opt, null);
                }
                if (!isset($val)) {
                        $val = true;    // enable i.e. --phpdoc without argument
                }
                switch ($key) {
                        case "-h":
                        case "--help":
                                usage($prog);
                                exit(0);
                        case '--constr':
                        case '--phpdoc':
                        case '--pretty':
                        case '--strict':
                                $opts[substr($key, 2)] = filter_var($val, FILTER_VALIDATE_BOOLEAN);
                                break;
                        case '--compact':
                                $opts['pretty'] = false;
                                break;
                        case '--access':
                        case '--member':
                                $opts[substr($key, 2)] = $val;
                                break;
                        default:
                                generate_classes((object) $opts, $opt);
                                break;
                }
        }
}

main($_SERVER['argc'], $_SERVER['argv']);
?>
