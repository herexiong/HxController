package main

import (
	"bufio"
	"fmt"
	"log"

	"gopkg.in/natefinch/npipe.v2"
)

func main() {
	pipeName := `\\.\pipe\MediaControlHubPipe_Test`
	ln, err := npipe.Listen(pipeName)
	if err != nil {
		log.Fatalf("Mock server failed to listen: %v", err)
	}
	defer ln.Close()

	fmt.Println("Mock HxMonitor Server listening on", pipeName)

	conn, err := ln.Accept()
	if err != nil {
		log.Fatalf("Accept error: %v", err)
	}

	fmt.Println("Proxy connected to Mock Server!")

	// Read from proxy
	reader := bufio.NewReader(conn)
	
	// Write a message to proxy
	mockMsg := `{"type":"cmd", "name":"Next"}` + "\n"
	conn.Write([]byte(mockMsg))
	fmt.Println("Sent mock message to proxy:", mockMsg)

	// Wait for a message from proxy
	line, err := reader.ReadString('\n')
	if err != nil {
		log.Printf("Error reading from proxy: %v", err)
	} else {
		fmt.Printf("Received message from proxy: %s", line)
	}

	conn.Close()
	fmt.Println("Mock Server shutting down.")
}
