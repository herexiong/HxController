package pipeclient

import (
	"io"
	"net"
	"time"

	"gopkg.in/natefinch/npipe.v2"
)

type Client struct {
	pipeName string
	conn     net.Conn
}

func New(pipeName string) *Client {
	return &Client{
		pipeName: pipeName,
	}
}

// Connect dial the named pipe. It blocks for timeout.
func (c *Client) Connect(timeout time.Duration) error {
	conn, err := npipe.DialTimeout(c.pipeName, timeout)
	if err != nil {
		return err
	}
	c.conn = conn
	return nil
}

// Write writes to the connected named pipe.
func (c *Client) Write(b []byte) (int, error) {
	if c.conn == nil {
		return 0, io.EOF
	}
	return c.conn.Write(b)
}

// Read reads from the connected named pipe.
func (c *Client) Read(b []byte) (int, error) {
	if c.conn == nil {
		return 0, io.EOF
	}
	return c.conn.Read(b)
}

// Close closes the connection.
func (c *Client) Close() error {
	if c.conn != nil {
		err := c.conn.Close()
		c.conn = nil
		return err
	}
	return nil
}
