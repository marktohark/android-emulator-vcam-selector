package main

import (
	"bufio"
	"net"
	"strconv"
	"strings"
	"sync"
)

type Context struct {
	conn             net.Conn
	Idx              int    `json:"-"`
	SelectedVCamName string `json:"selectedVCamName"`
	PName            string `json:"pName"`
	Pid              int    `json:"pid"`
}

type Communicate struct {
	ln               net.Listener
	connMap          sync.Map
	onSelectedChange func(int, string)
	onNewConnect     func(int, string)
	onUpdateConnList func(*sync.Map)

	selectedVCamName      string
	selectedVCamNameMutex sync.RWMutex
}

func NewCommunicate(OnNewConnect func(int, string), OnSelectedChange func(int, string), onUpdateConnList func(*sync.Map)) *Communicate {
	return &Communicate{
		onNewConnect:     OnNewConnect,
		onSelectedChange: OnSelectedChange,
		onUpdateConnList: onUpdateConnList,
	}
}

func (s *Communicate) Start() error {
	ln, err := net.Listen("tcp", "127.0.0.1:8693")
	if err != nil {
		return err
	}

	defer ln.Close()
	s.ln = ln

	idx := 0
	for {
		conn, err := ln.Accept()
		if err != nil {
			continue
		}
		go s.handleConn(idx, conn)
		idx++
	}
}

func (s *Communicate) Stop() {
	s.ln.Close()
	s.connMap.Range(func(key, value any) bool {
		ctx := value.(*Context)
		ctx.conn.Close()
		return true
	})
}

func (s *Communicate) SetVCamName(vcamName string) {
	s.selectedVCamNameMutex.Lock()
	defer s.selectedVCamNameMutex.Unlock()
	s.selectedVCamName = vcamName

	go func(vcamName string) {

		s.connMap.Range(func(key, value any) bool {
			ctx := value.(*Context)
			ctx.conn.Write([]byte(vcamName + ";"))
			return true
		})
	}(vcamName)
}

func (s *Communicate) handleConn(idx int, conn net.Conn) {
	ctx := &Context{
		conn: conn,
		Idx:  idx,
	}
	s.connMap.Store(idx, ctx)
	defer func() {
		conn.Close()
		s.connMap.Delete(idx)
		s.onUpdateConnList(&s.connMap)
	}()
	conn.Write([]byte(s.selectedVCamName + ";"))
	reader := bufio.NewReader(conn)
	var packages []string
	for {
		message, err := reader.ReadString(';')
		if err != nil {
			return
		}
		message = strings.TrimSuffix(message, ";")
		packages = append(packages, message)
		s.parsePackages(ctx, &packages)
	}
}

func (s *Communicate) parsePackages(ctx *Context, packages *[]string) {
	cmd := (*packages)[0]
	switch cmd {
	case "init":
		if len((*packages)) < 3 {
			return
		}
		pid, name := (*packages)[1], (*packages)[2]
		ctx.Pid, _ = strconv.Atoi(pid)
		ctx.PName = name
		*packages = (*packages)[3:]
		s.onNewConnect(ctx.Pid, ctx.PName)
		s.onUpdateConnList(&s.connMap)
	case "selected":
		if len((*packages)) < 2 {
			return
		}
		vcamName := (*packages)[1]
		ctx.SelectedVCamName = vcamName
		*packages = (*packages)[2:]
		s.onSelectedChange(ctx.Pid, ctx.SelectedVCamName)
	default:
		*packages = (*packages)[1:]
	}
}
