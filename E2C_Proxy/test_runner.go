package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"time"
)

func main() {
	fmt.Println("Starting Mock Server...")
	serverCmd := exec.Command("c:\\go\\bin\\go.exe", "run", "test_mock_server.go")
	serverCmd.Stdout = os.Stdout
	serverCmd.Stderr = os.Stderr
	if err := serverCmd.Start(); err != nil {
		log.Fatalf("Failed to start mock server: %v", err)
	}
	defer serverCmd.Process.Kill()

	time.Sleep(2 * time.Second)

	fmt.Println("Starting Proxy...")
	proxyCmd := exec.Command(".\\e2c_proxy.exe")
	proxyCmd.Stderr = os.Stderr
	stdin, _ := proxyCmd.StdinPipe()
	stdout, _ := proxyCmd.StdoutPipe()

	if err := proxyCmd.Start(); err != nil {
		log.Fatalf("Failed to start proxy: %v", err)
	}
	defer proxyCmd.Process.Kill()

	go func() {
		fmt.Println("Sending mock Chrome message to proxy stdin...")
		msg := []byte(`{"event":"status","state":"playing"}`)
		length := uint32(len(msg))
		binary.Write(stdin, binary.LittleEndian, length)
		stdin.Write(msg)
	}()

	fmt.Println("Reading from proxy stdout...")
	var respLen uint32
	err := binary.Read(stdout, binary.LittleEndian, &respLen)
	if err != nil {
		log.Fatalf("Failed to read header from proxy: %v", err)
	}
	
	respPayload := make([]byte, respLen)
	io.ReadFull(stdout, respPayload)
	fmt.Printf("Success! Proxy returned %d bytes: %s\n", respLen, string(respPayload))

	stdin.Close()
	time.Sleep(1 * time.Second)
}
