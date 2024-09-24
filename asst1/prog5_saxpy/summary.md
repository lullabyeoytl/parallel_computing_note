answer1:

只取得了(1.19x speedup from use of tasks)的加速效果

由于设计大规模数组运算->内存受限运算

-每次处理器处理数据时必须等待极长的内存访问时间，导致处理器利用率低下，从而降低了性能。通过并行计算并不改变决速步

answer2：

When the program writes to one element of result, it will first fetch the cache line which contains this element into the cache. This requires one memory operation. Then when this cache line is not needed, it will be flashed out of the cache, this requires another memory operation.

answer4:

也是上面内存受限运算的解决方案：减少数据访问规模

优化：

- SMID操作：使用了_m128类型和相关的指令来处理4个浮点数，提高性能

- 非临时储存（non-temporal memory）：通过指令将数据直接写入内存，（本来这里也不需要缓存）避免了缓存的影响，提高性能
 
 _mm_load_ps/_mm_store_ps：直接从内存加载/写入4个浮点数，避免了缓存的影响，提高性能
 _mm_stream_ps：将数据直接写入内存，不用等待缓存，提高性能