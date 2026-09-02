//go:build windows

package coral

import (
	"os"
	"os/exec"
	"path/filepath"

	"golang.org/x/sys/windows/registry"
)

// Paths holds the resolved locations of the Coral executables this
// server drives.
type Paths struct {
	CoralExe string
	CoralDiagExe  string
}

// Locate resolves the install locations of Coral.exe and fxdiag.exe.
//
// Resolution order: the installer's default Program Files location, the
// Windows uninstall registry (in case of a custom install directory), then
// PATH.
func Locate() (*Paths, error) {
	p := &Paths{}

	for _, dir := range candidateDirs() {
		if p.CoralExe == "" {
			if exe := filepath.Join(dir, "Coral.exe"); fileExists(exe) {
				p.CoralExe = exe
			}
		}
		if p.CoralDiagExe == "" {
			if exe := filepath.Join(dir, "fxdiag.exe"); fileExists(exe) {
				p.CoralDiagExe = exe
			}
		}
		if p.CoralExe != "" && p.CoralDiagExe != "" {
			break
		}
	}

	if p.CoralExe == "" {
		if exe, err := exec.LookPath("Coral.exe"); err == nil {
			p.CoralExe = exe
		}
	}
	if p.CoralDiagExe == "" {
		if exe, err := exec.LookPath("fxdiag.exe"); err == nil {
			p.CoralDiagExe = exe
		}
	}

	return p, validate(p)
}

func validate(p *Paths) error {
	var missing []string
	if p.CoralExe == "" {
		missing = append(missing, "Coral.exe")
	}
	if p.CoralDiagExe == "" {
		missing = append(missing, "fxdiag.exe")
	}
	if len(missing) > 0 {
		return newError(ErrKindAppNotFound, nil, "could not locate %v under Program Files, the Windows uninstall registry, or PATH", missing)
	}
	return nil
}

func fileExists(path string) bool {
	info, err := os.Stat(path)
	return err == nil && !info.IsDir()
}

// candidateDirs returns install directories to probe, in priority order:
// the installer's default Program Files location(s) (see
// Installer/coral.aip: Manufacturer="Coral", ProductName="Coral"),
// then any directory reported by the Windows uninstall registry for
// Coral, which covers a custom install directory chosen during setup.
func candidateDirs() []string {
	var dirs []string
	for _, envVar := range []string{"ProgramFiles", "ProgramFiles(x86)"} {
		if base := os.Getenv(envVar); base != "" {
			dirs = append(dirs, filepath.Join(base, "Coral", "Coral"))
		}
	}
	dirs = append(dirs, registryInstallDirs()...)
	return dirs
}

// registryInstallDirs scans the standard Windows uninstall registry keys
// for an entry whose DisplayName is Coral, returning its InstallLocation
// if found.
func registryInstallDirs() []string {
	var dirs []string
	roots := []struct {
		key  registry.Key
		path string
	}{
		{registry.LOCAL_MACHINE, `SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall`},
		{registry.LOCAL_MACHINE, `SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall`},
		{registry.CURRENT_USER, `SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall`},
	}
	for _, root := range roots {
		k, err := registry.OpenKey(root.key, root.path, registry.ENUMERATE_SUB_KEYS)
		if err != nil {
			continue
		}
		names, err := k.ReadSubKeyNames(-1)
		k.Close()
		if err != nil {
			continue
		}
		for _, name := range names {
			sk, err := registry.OpenKey(root.key, root.path+`\`+name, registry.QUERY_VALUE)
			if err != nil {
				continue
			}
			displayName, _, err := sk.GetStringValue("DisplayName")
			if err == nil && displayName == "Coral" {
				if loc, _, err := sk.GetStringValue("InstallLocation"); err == nil && loc != "" {
					dirs = append(dirs, loc)
				}
			}
			sk.Close()
		}
	}
	return dirs
}
