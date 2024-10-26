# saxpy 
input:

```bash
!chmod +x ./cudaSaxpy
!./cudaSaxpy -n 8092
```

output:
```
---------------------------------------------------------
Found 1 CUDA devices
Device 0: Tesla T4
   SMs:        40
   Global mem: 15102 MB
   CUDA Cap:   7.5
---------------------------------------------------------
Overall: 55.156 ms		[4.052 GB/s]
 2.452 ms		[91.145 GB/s]
 Total Bytes:240000000Overall: 59.047 ms		[3.785 GB/s]
 2.444 ms		[91.453 GB/s]
 Total Bytes:240000000Overall: 59.868 ms		[3.734 GB/s]
 2.450 ms		[91.248 GB/s]
 Total Bytes:240000000
```

input: 
```
!nvidia-smi```

output:
```
+---------------------------------------------------------------------------------------+
| NVIDIA-SMI 535.104.05             Driver Version: 535.104.05   CUDA Version: 12.2     |
|-----------------------------------------+----------------------+----------------------+
| GPU  Name                 Persistence-M | Bus-Id        Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |         Memory-Usage | GPU-Util  Compute M. |
|                                         |                      |               MIG M. |
|=========================================+======================+======================|
|   0  Tesla T4                       Off | 00000000:00:04.0 Off |                    0 |
| N/A   44C    P8               9W /  70W |      0MiB / 15360MiB |      0%      Default |
|                                         |                      |                  N/A |
+-----------------------------------------+----------------------+----------------------+
                                                                                         
+---------------------------------------------------------------------------------------+
| Processes:                                                                            |
|  GPU   GI   CI        PID   Type   Process name                            GPU Memory |
|        ID   ID                                                             Usage      |
|=======================================================================================|
|  No running processes found                                                           |
+---------------------------------------------------------------------------------------+
```

SMs: 40：这是指该显卡上有40个流式多处理器。每个SM能够同时执行大量的线程，这有助于提升并行计算的性能。

Global mem: 15102 MB：表示该显卡拥有15102 MB（约15 GB）的全局内存，这个内存用于存储程序运行时需要的数据。

CUDA Cap: 7.5：表示该设备的计算能力（Compute Capability）版本为7.5。这一版本信息不仅指示了显卡的硬件特性，也影响了代码的功能和性能，同时明确该GPU支持的CUDA功能

Tesla T4显卡的内存带宽为320 GB/s，而CPU 和 T4 GPU 之间的最大传输速度主要受 PCIe（外围组件互连高速接口）接口限制。NVIDIA T4 GPU 通常使用 PCIe 3.0 x16，其每个方向（CPU 到 GPU 和 GPU 到 CPU）的理论最大带宽约为 32 GB/s（千兆字节/秒）。

无论是4和32，91与320之间均有差距。

（tips from chatgpt）
actual transfer speeds may be lower due to overhead, system configuration, and other factors. If you're optimizing data transfer in your applications, consider using techniques like overlapping computation and data transfer or utilizing CUDA streams to improve efficiency.