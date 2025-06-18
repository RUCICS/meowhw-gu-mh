import matplotlib.pyplot as plt
import pandas as pd

# 读取数据
df = pd.read_csv("result.csv")

# 将 BlockSize 转为字符串作为横坐标标签（避免对数或数值缩放）
labels = df["BlockSize(Bytes)"].astype(str)
throughput = df["Throughput(MB/s)"]

# 绘图
plt.figure(figsize=(12, 6))
plt.plot(labels, throughput, marker='o', linestyle='-')

# 添加标题和标签
plt.title("Block Size vs Throughput")
plt.xlabel("Block Size (Bytes)")
plt.ylabel("Throughput (MB/s)")
plt.grid(True)
plt.xticks(rotation=45)  # 横轴标签倾斜，防止重叠
plt.tight_layout()

# 保存图像（可选）
plt.savefig("throughput_plot.png", dpi=300)

# 显示图像
plt.show()
