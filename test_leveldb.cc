//
// Created by lau on 2021/6/17.
//
#include <cassert>
#include <iostream>
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
  std::string value;
  std::string key1 = "testkey1";
  leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
  if (s.IsNotFound()) {
    std::cout << "can not found for key:" << key1 << std::endl;
    db->Put(leveldb::WriteOptions(), key1, "testvalue1");
  }
  s = db->Get(leveldb::ReadOptions(), key1, &value);
  if (s.ok()) {
    std::cout << "found key:" << key1 << ",value:" << value << std::endl;
  }
  s = db->Delete(leveldb::WriteOptions(), key1);
  if (s.ok()) {
    std::cout << "delete key success which key:" << key1 << std::endl;
  }
  s = db->Get(leveldb::ReadOptions(), key1, &value);
  if (s.IsNotFound()) {
    std::cout << "can not found after delete for key:" << key1 << std::endl;
  }

  delete db;

  return 0;
}