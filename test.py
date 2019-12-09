# hijack stdout
from System import Console
pythonEngine.Runtime.IO.SetOutput(Console.OpenStandardOutput(), System.IO.StreamWriter(Console.OpenStandardOutput()))

# get current path, 0 is for Renode directory, 1 for cwd
cwd = monitor.CurrentPathPrefixes[1]

monitor.Parse("i @scripts/single-node/polarfire-soc.resc")
monitor.Parse("sysbus LoadELF @" + cwd + "/artifacts/pse-freertos-lwip.elf")
monitor.Parse("logFile @" + cwd + "/artifacts/out.log")

# get the created machine
manager = Antmicro.Renode.Core.EmulationManager.Instance
emulation = manager.CurrentEmulation
machine = emulation.Machines[0]

# plug into its uart
def CharReceived(c):
    CharReceived.s += chr(c)
CharReceived.s = ""

machine["sysbus.mmuart0"].CharReceived += CharReceived

emulation.StartAll()
import time
time.sleep(2)

assert "PolarFire SoC Icicle FreeRTOS WebServer Demo" in CharReceived.s
assert "HTTP server is listening" in CharReceived.s
assert "Configuration done." in CharReceived.s

# make savepoint
Antmicro.Renode.Core.EmulationManager.Save(manager, cwd + "/artifacts/freertos.save")

#quit
monitor.Parse("q")
