///////////////////////////////////////////////////
//
// Copyright 2019 Broadcom Inc.
//
///////////////////////////////////////////////////

package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"rest/server"
	"swagger"
)

// Start REST server
func main() {
	var port int
	var uiDir string
	var useTransD bool

	flag.IntVar(&port, "port", 8080, "Listen port")
	flag.StringVar(&uiDir, "ui", "/usr/sonic-mgmt/ui", "UI directory")
	flag.BoolVar(&useTransD, "transd", false, "Connect to TransD")
	flag.Parse()

	swagger.Load()

	server.SetUIDirectory(uiDir)

	router := server.NewRouter()

	if useTransD {
		log.Printf("Will connect to TransD!")
		server.ConnectToTransD = true
	}

	address := fmt.Sprintf(":%d", port)
	log.Printf("Server started on %v", address)
	log.Printf("UI directory is %v", uiDir)

	log.Fatal(http.ListenAndServe(address, router))
}
