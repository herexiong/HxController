package main

import (
	"e2c_proxy/proxy"
)

func main() {
	// Disable logging to standard output as it disrupts Chrome's Native Messaging protocol
	// Any logs written generally will be either disabled or written to a temporary log file.
	
	// Create a log file in the temp directory (optional debugging)
	// logFile, _ := os.OpenFile(os.TempDir()+"\\e2c_proxy.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
	// if logFile != nil {
	// 	log.SetOutput(logFile)
	// 	defer logFile.Close()
	// } else {
	// 	log.SetOutput(io.Discard)
	// }
	
	// Pipe Name from the architecture document
	pipeName := `\\.\pipe\MediaControlHubPipe`
	
	// Start the proxy loop
	proxy.Run(pipeName)
	
	// Keep main alive, the exit happens in proxy.Run reading from os.Stdin
	select {}
}
