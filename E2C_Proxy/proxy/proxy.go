package proxy

import (
	"bufio"
	"bytes"
	"e2c_proxy/nativemsg"
	"e2c_proxy/pipeclient"
	"log"
	"os"
	"sync"
	"time"
)

func Run(pipeName string) {
	pipe := pipeclient.New(pipeName)

	// A channel to receive messages from Chrome
	chromeMsgs := make(chan []byte, 100)

	// Goroutine 1: Read from Chrome forever
	go func() {
		for {
			msg, err := nativemsg.ReadMessage(os.Stdin)
			if err != nil {
				// EOF or read error means Chrome disconnected us
				log.Printf("Failed to read from Chrome: %v", err)
				os.Exit(0) // Safe kill, no zombie
			}

			// Try to queue the message non-blocking or just drop it if channel is full
			select {
			case chromeMsgs <- msg:
			default:
				log.Println("Pipe disconnected or busy, dropping message from Chrome")
			}
		}
	}()

	var pipeMutex sync.Mutex
	pipeConnected := false

	// Main loop: Try to keep connection alive to HxMonitor
	for {
		err := pipe.Connect(time.Second)
		if err != nil {
			log.Printf("Failed to connect to pipe (HxMonitor offline): %v", err)
			time.Sleep(3 * time.Second)
			continue
		}

		log.Println("Connected to HxMonitor pipe!")

		pipeMutex.Lock()
		pipeConnected = true
		pipeMutex.Unlock()

		// Goroutine 2: Read from pipe and write to Chrome
		pipeReaderDone := make(chan struct{})
		go func() {
			defer close(pipeReaderDone)
			reader := bufio.NewReader(pipe)
			for {
				line, err := reader.ReadBytes('\n')
				if len(line) > 0 {
					// Remove \n if any, as Chrome Native Msg handles raw JSON object
					cleanMsg := bytes.TrimRight(line, "\r\n")
					if len(cleanMsg) > 0 {
						if err := nativemsg.WriteMessage(os.Stdout, cleanMsg); err != nil {
							log.Printf("Failed to write to Chrome: %v", err)
							os.Exit(0)
						}
					}
				}
				if err != nil {
					log.Printf("Pipe read error (HxMonitor disconnected): %v", err)
					return
				}
			}
		}()

		// Process Chrome messages while connected
		for pipeConnected {
			select {
			case msg := <-chromeMsgs:
				// Append newline to msg for pipe transmission
				msgWithLF := append(msg, '\n')
				_, wErr := pipe.Write(msgWithLF)
				if wErr != nil {
					log.Printf("Failed to write to pipe: %v", wErr)
					pipeMutex.Lock()
					pipeConnected = false
					pipeMutex.Unlock()
				}
			case <-pipeReaderDone:
				pipeMutex.Lock()
				pipeConnected = false
				pipeMutex.Unlock()
			}
		}

		// Reconnect gracefully after disconnect
		pipe.Close()
		time.Sleep(3 * time.Second)
	}
}
