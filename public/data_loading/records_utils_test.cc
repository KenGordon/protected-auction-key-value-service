// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "public/data_loading/records_utils.h"

#include "absl/hash/hash_testing.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace kv_server {
namespace {

KeyValueMutationRecordStruct GetKeyValueMutationRecord(
    KeyValueMutationRecordValueT value = "value") {
  KeyValueMutationRecordStruct record;
  record.key = "key";
  record.value = value;
  record.logical_commit_time = 1234567890;
  record.mutation_type = KeyValueMutationType::Update;
  return record;
}

UserDefinedFunctionsConfigStruct GetUdfConfigStruct(
    std::string_view code_snippet = "function my_handler(){}") {
  UserDefinedFunctionsConfigStruct udf_config_struct;
  udf_config_struct.language = UserDefinedFunctionsLanguage::Javascript;
  udf_config_struct.code_snippet = code_snippet;
  udf_config_struct.handler_name = "my_handler";
  udf_config_struct.logical_commit_time = 1234567890;
  return udf_config_struct;
}

DataRecordStruct GetDataRecord(RecordT record) {
  DataRecordStruct data_record_struct;
  data_record_struct.record = record;
  return data_record_struct;
}

TEST(KeyValueMutationRecordStructTest, ValidateEqualsOperator) {
  EXPECT_EQ(GetKeyValueMutationRecord(), GetKeyValueMutationRecord());
  EXPECT_NE(GetKeyValueMutationRecord("value1"),
            GetKeyValueMutationRecord("value2"));
  std::vector<std::string_view> values1{"value1", "value2"};
  EXPECT_EQ(GetKeyValueMutationRecord(values1),
            GetKeyValueMutationRecord(values1));
  std::vector<std::string_view> values2{"value3", "value4"};
  EXPECT_NE(GetKeyValueMutationRecord(values1),
            GetKeyValueMutationRecord(values2));
}

TEST(KeyValueMutationRecordStructTest, VerifyRecordStructValueIsEmpty) {
  KeyValueMutationRecordValueT value;
  EXPECT_TRUE(IsEmptyValue(value));
  value = "test";
  EXPECT_FALSE(IsEmptyValue(value));
  value = std::vector<std::string_view>{"test1", "test2"};
  EXPECT_FALSE(IsEmptyValue(value));
}

TEST(UdfConfigStructTest, ValidateEqualsOperator) {
  EXPECT_EQ(GetUdfConfigStruct(), GetUdfConfigStruct());
  EXPECT_NE(GetUdfConfigStruct("code_snippet1"),
            GetUdfConfigStruct("code_snippet2"));
}

TEST(DataRecordStructTest, ValidateEqualsOperator) {
  RecordT record;
  EXPECT_EQ(GetDataRecord(record), GetDataRecord(record));

  EXPECT_EQ(GetDataRecord(GetKeyValueMutationRecord()),
            GetDataRecord(GetKeyValueMutationRecord()));
  EXPECT_NE(GetDataRecord(GetKeyValueMutationRecord("value1")),
            GetDataRecord(GetKeyValueMutationRecord("value2")));
  std::vector<std::string_view> values1{"value1", "value2"};
  EXPECT_EQ(GetDataRecord(GetKeyValueMutationRecord(values1)),
            GetDataRecord(GetKeyValueMutationRecord(values1)));
  std::vector<std::string_view> values2{"value3", "value4"};
  EXPECT_NE(GetDataRecord(GetKeyValueMutationRecord(values1)),
            GetDataRecord(GetKeyValueMutationRecord(values2)));

  EXPECT_EQ(GetDataRecord(GetUdfConfigStruct()),
            GetDataRecord(GetUdfConfigStruct()));
  EXPECT_NE(GetDataRecord(GetUdfConfigStruct("code_snippet1")),
            GetDataRecord(GetUdfConfigStruct("code_snippet2")));
}

class RecordValueTest
    : public testing::TestWithParam<KeyValueMutationRecordValueT> {
 protected:
  KeyValueMutationRecordValueT GetValue() { return GetParam(); }
};

void ExpectEqual(const KeyValueMutationRecordStruct& record,
                 const KeyValueMutationRecord& fbs_record) {
  EXPECT_EQ(record.key, fbs_record.key()->string_view());
  EXPECT_EQ(record.logical_commit_time, fbs_record.logical_commit_time());
  EXPECT_EQ(record.mutation_type, fbs_record.mutation_type());
  if (fbs_record.value_type() == Value::String) {
    EXPECT_EQ(std::get<std::string_view>(record.value),
              GetRecordValue<std::string_view>(fbs_record));
  }
  if (fbs_record.value_type() == Value::StringSet) {
    EXPECT_THAT(std::get<std::vector<std::string_view>>(record.value),
                testing::ContainerEq(
                    GetRecordValue<std::vector<std::string_view>>(fbs_record)));
  }
}

void ExpectEqual(const UserDefinedFunctionsConfigStruct& record,
                 const UserDefinedFunctionsConfig& fbs_record) {
  EXPECT_EQ(record.language, fbs_record.language());
  EXPECT_EQ(record.logical_commit_time, fbs_record.logical_commit_time());
  EXPECT_EQ(record.code_snippet, fbs_record.code_snippet()->string_view());
  EXPECT_EQ(record.handler_name, fbs_record.handler_name()->string_view());
}

void ExpectEqual(const DataRecordStruct& record, const DataRecord& fbs_record) {
  if (fbs_record.record_type() == Record::KeyValueMutationRecord) {
    ExpectEqual(std::get<KeyValueMutationRecordStruct>(record.record),
                *fbs_record.record_as_KeyValueMutationRecord());
  }
  if (fbs_record.record_type() == Record::UserDefinedFunctionsConfig) {
    ExpectEqual(std::get<UserDefinedFunctionsConfigStruct>(record.record),
                *fbs_record.record_as_UserDefinedFunctionsConfig());
  }
}

INSTANTIATE_TEST_SUITE_P(RecordValueType, RecordValueTest,
                         testing::Values("value1",
                                         std::vector<std::string_view>{
                                             "value1", "value2"}));
TEST_P(RecordValueTest, VerifyDeserializeRecordToFbsRecord) {
  auto record = GetKeyValueMutationRecord(GetValue());
  testing::MockFunction<absl::Status(const KeyValueMutationRecord&)>
      record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&record](const KeyValueMutationRecord& fbs_record) {
        ExpectEqual(record, fbs_record);
        return absl::OkStatus();
      });
  auto status = DeserializeRecord(ToStringView(ToFlatBufferBuilder(record)),
                                  record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST_P(RecordValueTest, VerifyDeserializeRecordToRecordStruct) {
  auto record = GetKeyValueMutationRecord(GetValue());
  testing::MockFunction<absl::Status(const KeyValueMutationRecordStruct&)>
      record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&record](const KeyValueMutationRecordStruct& actual_record) {
        EXPECT_EQ(record, actual_record);
        return absl::OkStatus();
      });
  auto status = DeserializeRecord(ToStringView(ToFlatBufferBuilder(record)),
                                  record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest,
     DeserializeDataRecord_ToFbsRecord_KVMutation_StringValue_Success) {
  auto data_record_struct = GetDataRecord(GetKeyValueMutationRecord("value"));
  testing::MockFunction<absl::Status(const DataRecord&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecord& data_record_fbs) {
        ExpectEqual(data_record_struct, data_record_fbs);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest,
     DeserializeDataRecord_ToFbsRecord_KVMutation_StringVectorValue_Success) {
  std::vector<std::string_view> values({"value1", "value2"});
  auto data_record_struct = GetDataRecord(GetKeyValueMutationRecord(values));
  testing::MockFunction<absl::Status(const DataRecord&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecord& data_record_fbs) {
        ExpectEqual(data_record_struct, data_record_fbs);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest,
     DeserializeDataRecord_ToStruct_KVMutation_StringValue_Success) {
  auto data_record_struct = GetDataRecord(GetKeyValueMutationRecord("value"));
  testing::MockFunction<absl::Status(const DataRecordStruct&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecordStruct& actual_record) {
        EXPECT_EQ(data_record_struct, actual_record);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest,
     DeserializeDataRecord_ToStruct_KVMutation_VectorStringValue_Success) {
  std::vector<std::string_view> values({"value1", "value2"});
  auto data_record_struct = GetDataRecord(GetKeyValueMutationRecord(values));
  testing::MockFunction<absl::Status(const DataRecordStruct&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecordStruct& actual_record) {
        EXPECT_EQ(data_record_struct, actual_record);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest, DeserializeDataRecord_ToFbsRecord_UdfConfig_Success) {
  auto data_record_struct = GetDataRecord(GetUdfConfigStruct());
  testing::MockFunction<absl::Status(const DataRecord&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecord& data_record_fbs) {
        ExpectEqual(data_record_struct, data_record_fbs);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

TEST(DataRecordTest, DeserializeDataRecord_ToStruct_UdfConfig_Success) {
  auto data_record_struct = GetDataRecord(GetUdfConfigStruct());
  testing::MockFunction<absl::Status(const DataRecordStruct&)> record_callback;
  EXPECT_CALL(record_callback, Call)
      .WillOnce([&data_record_struct](const DataRecordStruct& actual_record) {
        EXPECT_EQ(data_record_struct, actual_record);
        return absl::OkStatus();
      });
  auto status = DeserializeDataRecord(
      ToStringView(ToFlatBufferBuilder(data_record_struct)),
      record_callback.AsStdFunction());
  EXPECT_TRUE(status.ok()) << status;
}

}  // namespace
}  // namespace kv_server
