/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>

#include <map>

#include <process/clock.hpp>
#include <process/future.hpp>
#include <process/statistics.hpp>

#include <stout/time.hpp>

using namespace process;

using std::map;


TEST(Statistics, set)
{
  Statistics statistics(seconds(60*60*24));

  statistics.set("statistic", 3.14);

  Future<map<seconds, double> > values = statistics.get("statistic");

  values.await();

  ASSERT_TRUE(values.isReady());
  EXPECT_EQ(1, values.get().size());
  EXPECT_GE(Clock::now(), values.get().begin()->first.value);
  EXPECT_DOUBLE_EQ(3.14, values.get().begin()->second);
}


TEST(Statistics, truncate)
{
  Clock::pause();

  Statistics statistics(seconds(60*60*24));

  statistics.set("statistic", 3.14);

  Future<map<seconds, double> > values = statistics.get("statistic");

  values.await();

  ASSERT_TRUE(values.isReady());
  EXPECT_EQ(1, values.get().size());
  EXPECT_GE(Clock::now(), values.get().begin()->first.value);
  EXPECT_DOUBLE_EQ(3.14, values.get().begin()->second);

  Clock::advance((60*60*24) + 1);

  statistics.increment("statistic");

  values = statistics.get("statistic");

  values.await();

  ASSERT_TRUE(values.isReady());
  EXPECT_EQ(1, values.get().size());
  EXPECT_GE(Clock::now(), values.get().begin()->first.value);
  EXPECT_DOUBLE_EQ(4.14, values.get().begin()->second);

  Clock::resume();
}
