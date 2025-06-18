#!/bin/bash

echo "BlockSize(Bytes),Throughput(MB/s)" > result.csv

for i in $(seq 9 20); do
    bs=$((2**i))
    count=1024
    
    output=$(LC_ALL=C dd if=/dev/zero of=/dev/null bs=$bs count=$count 2>&1)
    
    speed=$(echo "$output" | awk '
        /bytes.*copied/ {
            # 查找包含速度的字段
            for (i=1; i<=NF; i++) {
                if ($i ~ /[0-9.]+[GMK]?B\/s/) {
                    # 提取数值和单位
                    if (match($i, /([0-9.]+)([GMK]?B\/s)/, a)) {
                        value = a[1]
                        unit = a[2]
                        
                        # 转换为MB/s
                        if (unit == "GB/s") value *= 1000
                        else if (unit == "KB/s") value /= 1000
                        else if (unit == "B/s") value /= 1000000
                        
                        # 返回格式化结果
                        printf "%.2f", value
                        exit 0
                    }
                }
            }
            # 如果上面没找到，尝试匹配逗号分隔的速度部分
            if (match($0, /, ([0-9.]+) ([GMK]?B\/s)/, a)) {
                value = a[1]
                unit = a[2]
                
                if (unit == "GB/s") value *= 1000
                else if (unit == "KB/s") value /= 1000
                else if (unit == "B/s") value /= 1000000
                
                printf "%.2f", value
                exit 0
            }
        }
        END { if (!value) print "0.00" }
    ')
    
    echo "$bs,$speed" >> result.csv
    echo "bs=$bs -> $speed MB/s"
done
