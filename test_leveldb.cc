//
// Created by lau on 2021/6/17.
//
#include <cassert>
#include <iostream>
#include <ctime>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>

using namespace std;
using namespace leveldb;

int main()
{
  DB *db;
  Options options;
  options.create_if_missing = true;
  Status status = DB::Open(options, "./dbtest", &db);
  assert(status.ok());
  time_t start = time(nullptr);
  for (int i = 0; i < 800000; ++i) {
    db->Put(leveldb::WriteOptions(), std::to_string(i) + "test_key_", std::to_string(i)+"test_value_");
  }

  delete db;

  return 0;
}