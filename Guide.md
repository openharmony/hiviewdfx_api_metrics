# 一、直方图打点指南
## 1. 定义有用的指标
### 1.1 直接测量目标指标
推荐做法直接测量业务核心关注指标。例如，当需要评估函数总耗时时，若该函数调用了两个已完成直方图埋点的子函数直接相加两个子函数的直方图数据可能导致不准确的统计结果。这种错误源于直方图的预聚合特性 - 客户端对原始数据进行聚合处理，导致无法追溯具体调用实例的关联关系。将两个直方图数据相加等同于假设它们的完全独立，但实际场景中可能存在调用顺序或依赖关系。

### 1.2 提供背景信息
定义新指标时，**应提前考虑数据如何被分析**。很多情况下，需要提供上下文才能使数据可解释。

### 1.3 给直方图命名
**直方图使用点号（<span style="color:#e60000;">.</span>）作为分隔符来按类别组织**。名称应当为<span style="color:#e60000;"> KitName.Name</span> 或 <span style="color:#e60000;">KitName.Category.Name</span> 等形式，其中每个类别用于组织相关直方图。

通常不应随意添加新的顶层分类。如果确实需要，请先确认现有分类是否已涵盖该指标。

## 2. 选择直方图类型
各子系统可以根据自身场景来选择不同的直方图类型来进行打点。

| 直方图类型  | 接口定义  |说明|使用案例  |
|--|--|--|--|
|布尔直方图  |	#define HISTOGRAM_BOOLEAN(name, sample)  | 记录二元事件，如是否发生某个操作 |HISTOGRAM_BOOLEAN("ArkUI.APICall.createLongPressGesture", isCalled)|
|枚举直方图  | 	#define HISTOGRAM_ENUMERATION(name, sample, boundary) |记录用户行为、操作类型等枚举值|HISTOGRAM_ENUMERATION("ArkUI.ErrorCode.createLongPressGesture",ARKUI_ERROR_CODE_PARAM_ERROR,ArkUIErrorCode::kMaxValue)	  |
|计数直方图	  | #define HISTOGRAM_CUSTOM_COUNTS(name, sample, min, max, bucket_count)|	记录范围较大的计数数据 |HISTOGRAM_CUSTOM_COUNTS("ArkUI.Button.click", 3, 1, 100, 10)	  |
|时间直方图  | #define HISTOGRAM_TIMES(name, sample) | 记录时间间隔，单位ms，如长按时间等 |HISTOGRAM_TIMES("ArkUI.Button.longPress", 2)	|
|百分比直方图  |  #define HISTOGRAM_PERCENTAGE(name, sample)| 记录百分比，如完成度等 |HISTOGRAM_PERCENTAGE("ArkUI.ImageCompression.Ratio", compression_ratio_percentage)	|

### 2.1 布尔直方图
布尔直方图适合用于记录二元事件的分析。例如某个API是否调用以及调用次数。

### 2.2 枚举直方图
枚举直方图适合用于一组相关状态的联合分析。例如API调用返回错误码很适合作为枚举直方图。 如果直方图的总计数（所有桶之和）有意义，通常说明枚举直方图是合适的选择。

枚举直方图也适合用于计数事件。例如想统计API调用返回不同错误码的个数。

### 2.3 计数直方图
计数直方图需要自行选择最小值、最大值和桶数，请根据下文建议谨慎选择数量。

选择最小和最大值 最大值应保证很少有样本超过 max。若样本值 >= max，会落入“溢出桶”。若溢出桶过大统计会困难。经验上，溢出桶占比最好 <= 1%（越小越好），这样可以分析 99 分位。宁可稍大，也不要太小。若设置不当，需要等下一个发布周期才能修复。 最小值：若应用场景需要包含 0 值由于直方图已包含 0 的下溢桶，因此 min=1 可有效覆盖非 0 范围。否则请根据实际选择最小值。

选择桶数量 选择满足粒度需求的最小桶数。计数直方图桶大小默认按指数递增，小值粒度更细，大值仍有合理分辨率。宏默认 50 个桶（大范围为 100），通常已足够。直方图会预分配所有桶，桶数直接影响内存占用。没有充分理由不要超过 100 个桶。

### 2.4 时间直方图
时间直方图可以用来记录函数调用时长（TimeDelta）。

### 2.5 百分比直方图
百分比直方图记录任意比例（等宽桶） 。

## 3. 直方图生命周期
### 3.1 新增直方图
第一步：定义有用的指标

明确需求并制定数据采集计划。
- 为什么需要这个指标？
- 该指标何时发布？
- 该指标有何价值？
- 查看仪表盘中的图表后，有哪些可以改进的地方？

- 为什么需要这个指标？
	- 追踪用户如何退出反馈应用程序
- 该指标何时发布？
	- 当用户关闭反馈应用时触发
- 该指标有何价值？
	- 确定提交率与取消率
	- 了解用户在流程中流失的位置
- 看完数据后，有哪些可以改进的地方？
	- 如果kQuitNoHelpContentClicked值较高 -> 这可能表明帮助内容没有用处，因此应该提高帮助内容文章的质量。
	- 如果kQuitNoResultFound（离线或搜索功能已关闭）值较高 -> 提高搜索可靠性，提醒用户在提交反馈时保持在线状态等。
步骤二：创建指标

- 选择直方图类型，常见类型有：
	- 枚举直方图
	- 布尔直方图
	- 时间直方图
- 在 histograms.xml 文件中需要的直方图类型
例如，TestEnum是一个枚举直方图。

```
<histogram name="a.testkit.testapi1" enum="TestEnum" expires_after="8.0.0.100">
  <data_type>1</data_type>
  <data_value>@hms.core.account.retailauth.d.ts#retailAuth#login</data_value>
  <metric_type>CALL_COUNTS</metric_type>
  <metric_requirements>VALUE_FEATURE</metric_requirements>
  <summary>
    Counting how many times this api used.
  </summary>
</histogram>
```

对于枚举类型，请同时添加一个枚举值。
```
<enum name="TestEnum">
  <int value="0" label="kQuitSearchPageHelpContentClicked"/>
  <int value="1" label="kQuitSearchPageNoHelpContentClicked"/>
  <int value="2" label="kQuitNoResultFound"/>
  <int value="3" label="kQuitShareDataPageHelpContentClicked"/>
  <int value="4" label="kQuitShareDataPageNoHelpContentClicked"/>
  <int value="5" label="kSuccessHelpContentClicked"/>
  <int value="6" label="kSuccessNoHelpContentClicked"/>
</enum>
```

在 histogram_util.cc 中调用打点函数
```
void EmitFeedbackAppExitPath(mojom::FeedbackAppExitPath exit_path) {
    HISTOGRAM_ENUMERATION(kFeedbackAppExitPath, exit_path);
}
```

### 3.2 修改直方图
当修改直方图的语义（例如，何时生成直方图、桶代表什么、数值直方图的桶范围或桶数等）时，请创建一个具有新名称的新直方图。如果用户不会在桶之间移动，且桶比例没有意义，则向枚举添加新值时无需创建新的直方图名称。否则，混合更改前后的数据进行分析可能会产生误导。

在某些情况下，即使语义完全没有改变，也允许对直方图进行更改。以下是一些允许更改的示例：
- 可以对直方图摘要进行重写，使其更加准确。
枚举桶的标签可以更改，只要它仍然指代与之前相同的事物即可。

### 3.3 废弃直方图
废弃说明
在 histograms.xml 的直方图描述中，通过_after 属性指定过期，该属性是必填项。可以用 YYYY-MM-DD 日期格式或 API(例如 API7.0.1)里程碑格式。 过期后记录该直方图的代码将被视为无效代码，应从代码库删除；同时应清理 histograms.xml 的条。在特殊场景下，可将过期设置为 "never"，用于标记长期保留的关键指标（通常用于跨项目监控指标）。
。

常见场景建议
- 如果 owner 和团队都不再使用该直方图，应删除它， 如果当前不用、但存在未来使用可能，仍建议删除它。
- 如果当前不用、将来可能用，删除它。
- 如果当前不用，但近期可能用，设置 ~3 个月后过期。
- 否则，设置合理的过期时间，最长不超过 1 年。
结论是：若直方图在被查看，应自动延长，无需开发者介入。

**如何为新直方图选择过期时间**
一般应选择合理的使用期限，最长不超过 1 年。

常见情况：
- 若为评估新功能上线，过期时间与预期功能上线时间一致。
- 若预计直方图长期有用，设置不超过 1 年的过期时间，以便之后重新评估是否确实有用。
- 否则通常设置 3-6 个月。

### 3.4 删除直方图
请删除不再需要的直方图相关的代码，直方图会占用内存。

## 4. 高性能打点
在典型场景下，直方图点的性能开销通常低于1us。

但是如果您在对性能要求特别高或“热点”的代码中记录直方图，当总数并不重要时（例如，测量延迟或比率时），可以考虑：
```
if ( base :: ShouldRecordSubsampledMetric ( 0.01 )) { 
    HISTOGRAM_TIMES ( "Component.Feature.Duration.Subsampled" , timer- > Elapsed ()); 
}
```
例如，对于应用于每一帧都需要打点的直方图，就可以进行这些优化。

## 5. 测试直方图
### 5.1 本地测试
进入hdc shell，提供命令行来导出直方图打点数据
```
hidumper -s 66568 -a all
```
### 5.2 端到端远程测试
HA采集数据上报到测试环境，进行数据查看
![image.png](https://raw.gitcode.com/user-images/assets/9605947/76a0cca4-6685-4698-8caa-95559fa2d17e/image.png 'image.png')

# 二、审核直方图
## 1. 如何成为审核员
API Committer 默认为各自领域打点项审核人员。

## 2. 审核规范
### 2.1 安全隐私
打点项数据为匿名统计，不会追踪到用户个人，因此HA现有协议能够包住。

### 2.2 正确性
直方图记录是否正确？

请检查是否存在错误和未使用的桶。
存储桶布局是否高效？通常情况下，如果存储桶数量超过 50 个，就应该提出异议。
对于计时指标，最小值和最大值范围对于所测量的持续时间是否合理？
### 2.3 可持续性
添加的指标/存储桶数量是否合理？
虽然没有硬性规定，但任何超过 20 个独立直方图的情况都应该升级处理。
任何超过 100 个可能桶的直方图都应该进行升级。
有效期设定是否合理？

# 三、打点原则
## 1. 高价值特性类强制打点
高价值特性(如闪控球、字体使用等)功能接口需要业务需求 (运维、特性活跃度)打点，用于统计特性活跃度、运维等

## 2. 错误码、失败率等按需打点
## 3. 对于性能监控、安全和变更废弃有监控需求的，按需进行打点；
用于统计性能、接口变更废弃参考数据

## 4. 对性能要求较高的接口（可能被高频调用，如按帧），谨慎使用业务需求打点；

# 四、FAQ
## 问题1，大量接口接入打点，是否会达到上报数量的限制？
控制到5000-6000个打点项

## 问题2，接口性能？
本地测试1us以下，环境正常情况下测试可以达到ns

## 问题3，从信息统计维度，框架上报的信息里面是否包括：设备、版本、应用信息、接口（打点项）
按照打点项统计、beta、特性看板，可以通过设备、版本、应用信息来过滤
对于高频接口：提供设置采样率，如一个接口调100次只打一次点

## 问题4，不同类型的打点接口相同吗？如性能打点，每次的性能不一样，怎么统计？Error code也类似
getWindow [0-100ms] :11次, [100-200]:23次,[1000-无穷]
有固定档位和可配置档位

## ## 
问题5，两套打点接口以后的分工是什么？Hisysevent打了是否还需要在新功能中打点？
新架构偏向统计意义的打点。


## 问题6，看板是否需要自己配置
数据格式统一的，会有统一的直方图看板。可以根据版本、设备等过滤