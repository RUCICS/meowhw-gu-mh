import matplotlib.pyplot as plt

# 程序名称
programs = [
    "mycat1", "mycat2", "mycat3",
    "mycat4", "mycat5", "mycat6"
]

# 实际从 hyperfine 输出提取的 mean 时间
times = [
    642.374,   # mycat1: 字符逐个读写
    0.3571,    # mycat2: 页大小缓冲
    0.3117,    # mycat3: 页对齐
    0.3213,    # mycat4: 加入块大小考虑
    0.2323,    # mycat5: 使用最佳缓冲区大小
    0.2371     # mycat6: 使用 fadvise 优化
]

# 绘图
plt.figure(figsize=(10, 6))
bars = plt.bar(programs, times, color='cornflowerblue')

# 标注每个柱子的数值
for bar, t in zip(bars, times):
    plt.text(bar.get_x() + bar.get_width() / 2, t + 0.02, f"{t:.3f}s", ha='center')

plt.ylabel("Execution Time (seconds)")
plt.title("Execution Time of mycat1~mycat6")
plt.ylim(0, max(times) + 50)
plt.grid(axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig("conclusion_plot.png", dpi=300)
plt.show()
