package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"math/rand"
	"net"
	"sync"
	"time"
)

func write(conn net.Conn, data []byte) {
	l := make([]byte, 4)
	var b []byte = data
	n := uint32(len(b))
	binary.BigEndian.PutUint32(l, n)
	w := bufio.NewWriter(conn)
	if _, err := w.Write(l); err != nil {
		fmt.Printf("session: Codec write header error %v", err)
		return
	}
	if _, err := w.Write(b); err != nil {
		fmt.Printf("session: Codec write body error %v", err)
		return
	}
	w.Flush()
}

var chars string = "abcdefghijklmnopqrstuvwxzy1234567890!@#$5^&*()_+./,ABCFEFGILJKLMNOQ~!-*?XASVBCGFGFJYTUYTK"

func pingPing(ii int) {
	conn, err := net.Dial("tcp", "192.168.11.223:1234")
	if err != nil {
		fmt.Printf("tcp dial failed, err:%s\n", err.Error())
		return
	}

	write(conn, []byte("hello"))

	defer conn.Close()

	rand.Seed(time.Now().UnixNano())

	//r := bufio.NewReader(conn)
	go func() {
		charslen := len(chars)
		for ; ; {
			var data = []byte("ping pingpingpingping")
			for i := 0; ; i++ {
				data = append(data, chars[(i+rand.Int())%charslen])
				if rand.Float64() < 0.02 {
					break
				}
			}
			write(conn, data)
			time.Sleep(10 *time.Millisecond)
		}
	}()

	for {
		{
			h := make([]byte, 100)
			if _, err := conn.Read(h); err != nil {
				fmt.Println("remote close")
				return
			}
			fmt.Println(ii, string(h))
		}
	}
}

func main() {
	//pingPing(0)

	wr := sync.WaitGroup{}
	for i := 0; i < 50; i++ {
		wr.Add(1)
		go func() {
			pingPing(i)
			wr.Add(-1)
		}()
	}

	wr.Wait()
}
