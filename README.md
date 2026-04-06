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

# 打点规范 
打点规范细则请参考：[打点规范](https://gitcode.com/Bigdemon/hiviewdfx_api_metrics/edit/master/standard.md)


# 许可说明
参见对应目录的LICENSE文件及代码声明