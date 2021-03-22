// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#ifndef __PROCESS_STATISTICS_HPP__
#define __PROCESS_STATISTICS_HPP__

#include <glog/logging.h>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include <process/timeseries.hpp>

#include <stout/foreach.hpp>
#include <stout/option.hpp>

namespace process {

// Represents statistics for a `TimeSeries` of data or a standard container.
template <typename T>
struct Statistics
{
  // Returns `Statistics` for the given `TimeSeries`, or `None` if the
  // `TimeSeries` has less then 2 datapoints.
  //
  // TODO(dhamon): Consider adding a histogram abstraction for better
  // performance.
  //
  // Remove this specification once we can construct directly from
  // `TimeSeries<T>::Value`, e.g., by using an iterator adaptor, see
  // https://www.boost.org/doc/libs/1_51_0/libs/range/doc/html/range/reference/adaptors/reference/map_values.html // NOLINT(whitespace/line_length)
  static Option<Statistics<T>> from(const TimeSeries<T>& timeseries)
  {
    std::vector<typename TimeSeries<T>::Value> values_ = timeseries.get();

    std::vector<T> values;
    values.reserve(values_.size());

    foreach (const typename TimeSeries<T>::Value& value, values_) {
      values.push_back(value.data);
    }

    return from(std::move(values));
  }

  // Returns `Statistics` for the given container, or `None` if the container
  // has less then 2 datapoints. The container is represented as a pair of
  // [first, last) iterators.
  //
  // TODO(alexr): Consider relaxing the collection type requirement to
  // `std::is_convertible<std::iterator_traits<It>::value_type, T>`.
  template <
      typename It,
      typename = typename std::enable_if<
          std::is_same<
              typename std::iterator_traits<It>::value_type,
              T>::value &&
          std::is_convertible<
              typename std::iterator_traits<It>::iterator_category,
              std::forward_iterator_tag>::value>::type>
  static Option<Statistics<T>> from(It first, It last)
  {
    // Copy values into a vector.
    std::vector<T> values;
    values.reserve(std::distance(first, last));

    std::copy(first, last, std::back_inserter(values));

    return from(std::move(values));
  }

  size_t count;

  T min;
  T max;

  // TODO(dhamon): Consider making the percentiles we store dynamic.
  T p25;
  T p50;
  T p75;
  T p90;
  T p95;
  T p99;
  T p999;
  T p9999;

private:
  // Calculates `Statistics` from the provided vector; note pass by reference.
  static Option<Statistics<T>> from(std::vector<T>&& values)
  {
    // We need at least 2 values to compute aggregates.
    if (values.size() < 2) {
      return None();
    }

    std::sort(values.begin(), values.end());

    Statistics statistics;

    statistics.count = values.size();

    statistics.min = values.front();
    statistics.max = values.back();

    statistics.p25 = percentile(values, 0.25);
    statistics.p50 = percentile(values, 0.5);
    statistics.p75 = percentile(values, 0.75);
    statistics.p90 = percentile(values, 0.90);
    statistics.p95 = percentile(values, 0.95);
    statistics.p99 = percentile(values, 0.99);
    statistics.p999 = percentile(values, 0.999);
    statistics.p9999 = percentile(values, 0.9999);

    return statistics;
  }

  // Returns the requested percentile from the sorted values.
  // Note that we need at least two values to compute percentiles!
  //
  // TODO(dhamon): Use a 'Percentage' abstraction.
  static T percentile(const std::vector<T>& values, double percentile)
  {
    CHECK_GE(values.size(), 2u);

    if (percentile <= 0.0) {
      return values.front();
    }

    if (percentile >= 1.0) {
      return values.back();
    }

    // Use linear interpolation.
    const double position = percentile * (values.size() - 1);
    const size_t index = static_cast<size_t>(floor(position));
    const double delta = position - index;

    CHECK_GE(index, 0u);
    CHECK_LT(index, values.size() - 1);

    return values[index] + (values[index + 1] - values[index]) * delta;
  }
};

} // namespace process {

#endif // __PROCESS_STATISTICS_HPP__
