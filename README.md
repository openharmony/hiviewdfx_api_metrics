# hiviewdfx_api_metrics

API Metrics SIG仓，提供支持API直方图高性能低功耗打点功能，提供了计数、枚举、时间、百分比、布尔五种直方图

# 目录
```
hiviewdfx_api_metrices/
├── BUILD.gn                      # 项目构建配置
├── bundle.json                   # Bundle配置
├── hiviewdfx.gni                 # GN构建入口
├── manager/
│   ├── histogram/               # 直方图核心实现
│   │   ├── include/
│   │   │   ├── bucket_ranges.h          # 桶范围定义（线性/指数分桶）
│   │   │   ├── histogram_base.h         # 直方图抽象基类
│   │   │   └── shared_histogram.h       # 5种直方图类定义
│   │   └── src/
│   │       ├── bucket_ranges.cpp        # 桶范围实现
│   │       ├── histogram_base.cpp       # 基类空实现
│   │       └── shared_histogram.cpp     # 5种直方图实现
│   └── innerkits/               # 插件接口层
│       ├── include/
│       │   ├── histogram_plugin_macros.h # 直方图宏(现为内联函数)
│       │   ├── ihistogram_plugin.h       # 插件接口定义
│       │   ├── log_wrapper.h             # 日志封装
│       │   ├── plugin_interface.h        # 插件接口类
│       │   └── plugin_manager.h          # 插件管理器
│       └── src/
│           ├── plugin_interface.cpp      # 插件接口实现
│           └── plugin_manager.cpp        # 插件加载管理
└── test/                        # 测试demo目录
```
# 仓库功能
- 提供API运行时打点的规范、云侧上传规范、接口说明、示例代码
- 提供直方图打点接接口能力（inner API）
- 支持基于典型统计模型进行打点数据模型统计缓存（布尔直方图、枚举直方图、计数直方图、时间统计模型）
- 支持直方图统计数据本地持久化
- 支持设备范围：富设备

# 逻辑架构
- 打点策略管理：支持API 打点的新增、废弃管理，1阶段支持通过代码仓配置打点策略、2阶段支持云控

- 模型层：提供直方图、分桶区间、打点样本三个类型数据结构。

- 快照管理器：收集直方图数据，准备这些数据的增量，并将其记录在缓存中
- 内存分配器：用于多进程共享内存分配和直方图模型内存分配。
- 打点持久化管理：基于持久化策略触发缓存中的统计数据持久化到本地数据库中。定期对数据库中的打点数据进行老化处理


# 打点规范 
打点规范细则请参考：[打点规范](https://gitcode.com/Bigdemon/hiviewdfx_api_metrics/edit/master/standard.md)


# 许可说明
参见对应目录的LICENSE文件及代码声明