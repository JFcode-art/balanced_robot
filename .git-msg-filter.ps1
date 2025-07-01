$input = [Console]::In.ReadToEnd()
if ($input -match "Initial") { "初始提交：STM32F407 平衡机器人项目" }
elseif ($input -match "Add QT") { "添加 QT 上位机和蓝牙遥控功能" }
elseif ($input -match "Hardware") { "硬件修复：TIM冲突、I2C迁移、PID优化" }
elseif ($input -match "review") { "代码审查修复：USART RX模式、缓冲区、互斥锁、协议" }
else { $input }
