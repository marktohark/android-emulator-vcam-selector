package main

import (
	"bytes"
	"cmp"
	"context"
	"github.com/wailsapp/wails/v2/pkg/runtime"
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"slices"
	"strings"
	"sync"
)

// App struct
type App struct {
	ctx  context.Context
	comm *Communicate
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx

	a.comm = NewCommunicate(func(pid int, pName string) {
		runtime.EventsEmit(a.ctx, "new_connection", pid, pName)
	}, func(pid int, vcamName string) {
		runtime.EventsEmit(a.ctx, "selected_vcam", pid, vcamName)
	}, func(connMap *sync.Map) {
		var l []*Context
		connMap.Range(func(key, value any) bool {
			_ctx := value.(*Context)
			l = append(l, _ctx)
			return true
		})
		slices.SortFunc(l, func(a, b *Context) int {
			return cmp.Compare(a.Idx, b.Idx)
		})
		runtime.EventsEmit(a.ctx, "updated_list", l)
	})
	go func() {
		err := a.comm.Start()
		if err != nil {
			log.Fatalln(err)
		}
	}()
}

func (a *App) LsCamera() []string {
	cmd := exec.Command("./LsCamera.exe")
	var stdout, stderr bytes.Buffer
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr
	cmd.Run()
	if stderr.String() != "" {
		return []string{}
	}
	s := strings.TrimSuffix(stdout.String(), "\r\n")
	return strings.Split(s, "\r\n")
}

func (a *App) Apply(vcamName string) {
	a.comm.SetVCamName(vcamName)
}

func (a *App) Inject() []string {
	cameraExePath, _ := runtime.OpenFileDialog(a.ctx, runtime.OpenDialogOptions{
		Filters: []runtime.FileFilter{
			runtime.FileFilter{DisplayName: "camera (*.exe)", Pattern: "*.exe"},
		},
	})
	if cameraExePath == "" {
		return []string{"", ""}
	}

	currPath, _ := os.Executable()
	currPath = filepath.Dir(currPath)
	N9DllPath := path.Join(currPath, "An9VCamPicker.dll")
	setdllPath := path.Join(currPath, "setdll.exe")
	_, err := os.Stat(setdllPath)
	if os.IsNotExist(err) {
		return []string{"", "cannot find setdll.exe"}
	}
	cmd := exec.Command(setdllPath, "/d:"+N9DllPath, cameraExePath)
	var stdout, stderr bytes.Buffer
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr
	cmd.Run()
	return []string{stdout.String(), stderr.String()}
}

func (a *App) UnInject() []string {
	cameraExePath, _ := runtime.OpenFileDialog(a.ctx, runtime.OpenDialogOptions{
		Filters: []runtime.FileFilter{
			runtime.FileFilter{DisplayName: "camera (*.exe)", Pattern: "*.exe"},
		},
	})
	if cameraExePath == "" {
		return []string{"", ""}
	}
	currPath, _ := os.Executable()
	currPath = filepath.Dir(currPath)
	setdllPath := path.Join(currPath, "setdll.exe")
	_, err := os.Stat(setdllPath)
	if os.IsNotExist(err) {
		return []string{"", "cannot find setdll.exe"}
	}
	cmd := exec.Command(setdllPath, "/r", cameraExePath)
	var stdout, stderr bytes.Buffer
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr
	cmd.Run()
	return []string{stdout.String(), stderr.String()}
}
