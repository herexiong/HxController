package nativemsg

import (
	"encoding/binary"
	"io"
)

// ReadMessage reads a message from standard input with a 4-byte length prefix.
func ReadMessage(r io.Reader) ([]byte, error) {
	var length uint32
	// Default byte order for Native Messaging is little-endian
	if err := binary.Read(r, binary.LittleEndian, &length); err != nil {
		return nil, err
	}

	msg := make([]byte, length)
	if _, err := io.ReadFull(r, msg); err != nil {
		return nil, err
	}
	return msg, nil
}

// WriteMessage writes a message to standard output with a 4-byte length prefix.
func WriteMessage(w io.Writer, msg []byte) error {
	length := uint32(len(msg))
	if err := binary.Write(w, binary.LittleEndian, length); err != nil {
		return err
	}

	if _, err := w.Write(msg); err != nil {
		return err
	}
	return nil
}
