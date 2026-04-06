/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "shared_histogram.h"

#include <algorithm>
#include <cmath>
#include "bucket_ranges.h"

namespace OHOS::histogram {

// --- Histogram 实现 ---

Histogram::Histogram()
    : HistogramBase("", 0, 0, 0) // 调用基类默认初始化
{}

Histogram::Histogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : HistogramBase(name, minimum, maximum, bucket_count)
{
    Histogram::Initialize();
}

void Histogram::Initialize()
{
    bucket_ranges_ = std::make_unique<BucketRanges>(bucket_count_ + 1);
    bucket_ranges_->InitializeExponentialBuckets(minimum_, maximum_, bucket_count_);
    counts_.assign(bucket_count_ + 1, 0); // 使用 assign 确保重新初始化
}

void Histogram::Add(int32_t value)
{
    WriteToBucket(value);
}

void Histogram::WriteToBucket(int32_t value)
{
    if (!bucket_ranges_) {
        return;
    }
    
    int bucket_index = bucket_ranges_->GetBucketIndex(value);
    if (bucket_index < 0 || bucket_index >= static_cast<int>(counts_.size())) {
        return;
    }
    
    counts_[bucket_index]++;
    total_count_.fetch_add(1);  // 原子增加
    sum_.fetch_add(value);      // 原子增加
}

int64_t Histogram::TotalCount() const
{
    return total_count_.load();
}

int64_t Histogram::Sum() const
{
    return sum_.load();
}

// --- LinearHistogram 实现 ---

LinearHistogram::LinearHistogram()
    : Histogram("", 0, 0, 0)
{}

LinearHistogram::LinearHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : Histogram(name, minimum, maximum, bucket_count) // 透传给父类
{
    // 由于父类构造函数会调用它版本的 Initialize()，
    // 这里需要显式调用线性版本的初始化
    LinearHistogram::Initialize();
}

void LinearHistogram::Initialize()
{
    bucket_ranges_ = std::make_unique<BucketRanges>(bucket_count_ + 1);
    bucket_ranges_->InitializeLinearBuckets(minimum_, maximum_, bucket_count_);
    counts_.assign(bucket_count_ + 1, 0);
}

// --- BooleanHistogram 实现 ---

BooleanHistogram::BooleanHistogram()
    : LinearHistogram("", 0, 0, 0)
{}

BooleanHistogram::BooleanHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : LinearHistogram(name, minimum, maximum, bucket_count + 1)
{
    // 这里会自动继承 LinearHistogram 的 Initialize() 逻辑
}

// --- EnumHistogram 实现 ---

EnumHistogram::EnumHistogram()
    : LinearHistogram("", 0, 0, 0)
{}

EnumHistogram::EnumHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : LinearHistogram(name, minimum, maximum, bucket_count + 1)
{}

// --- CountHistogram 实现 ---

CountHistogram::CountHistogram()
    : Histogram("", 0, 0, 0)
{}

CountHistogram::CountHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : Histogram(name, minimum, maximum, bucket_count + 1)
{}

// --- TimeHistogram 实现 ---

TimeHistogram::TimeHistogram()
    : Histogram("", 0, 0, 0)
{}

TimeHistogram::TimeHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : Histogram(name, minimum, maximum, bucket_count + 1)
{}

// --- PercentageHistogram 实现 ---

PercentageHistogram::PercentageHistogram()
    : LinearHistogram("", 0, 0, 0)
{}

PercentageHistogram::PercentageHistogram(const std::string &name, int32_t minimum, int32_t maximum, size_t bucket_count)
    : LinearHistogram(name, minimum, maximum, bucket_count + 1)
{}

}  // namespace OHOS::histogram