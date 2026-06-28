param(
    [string]$ExePath = "notepad.exe"
)

$sw = [System.Diagnostics.Stopwatch]::StartNew()

# 2. 启动进程
$proc = Start-Process -FilePath $ExePath -PassThru
# $proc = Start-Process -FilePath $ExePath -PassThru -NoNewWindow

# 3. 等待进程结束
$proc.WaitForExit()

# 4. 停止计时
$sw.Stop()

# 5. 输出结果（等价于 Linux time）
[PSCustomObject]@{
    Process   = $proc.ProcessName
    PID       = $proc.Id
    ExitCode  = $proc.ExitCode
    RealTime  = $sw.Elapsed
    UserTime  = $proc.UserProcessorTime
    SysTime   = $proc.PrivilegedProcessorTime
    TotalCPU  = $proc.TotalProcessorTime
}
