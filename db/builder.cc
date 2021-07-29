// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/builder.h"

#include "db/dbformat.h"
#include "db/filename.h"
#include "db/table_cache.h"
#include "db/version_edit.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"

namespace leveldb {

Status BuildTable(const std::string& dbname, Env* env, const Options& options,
                  TableCache* table_cache, Iterator* iter, FileMetaData* meta) {
  Status s;
  meta->file_size = 0;
  // 移动到 SkipList 中第一个元素的位置
  iter->SeekToFirst();

  // 创建文件
  std::string fname = TableFileName(dbname, meta->number);
  if (iter->Valid()) {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    TableBuilder* builder = new TableBuilder(options, file);
    // SkipList 中的数据是按照顺序排列的
    // 因此第一个键就是其最小的键
    meta->smallest.DecodeFrom(iter->key());
    Slice key;
    // 不断地将 SkipList 中的数据插入到 TableBuilder 中
    for (; iter->Valid(); iter->Next()) {
      key = iter->key();
      builder->Add(key, iter->value());
    }
    // 最后一个键就是最大的键
    if (!key.empty()) {
      meta->largest.DecodeFrom(key);
    }

    // Finish and check for builder errors
    // Finish 时会写入索引部分的数据
    // 包括 FilterBlock、MetaIndexBlock、IndexBlock 以及 Footer
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      assert(meta->file_size > 0);
    }
    delete builder;

    // Finish and check for file errors
    if (s.ok()) {
      s = file->Sync();
    }
    if (s.ok()) {
      s = file->Close();
    }
    delete file;
    file = nullptr;

    if (s.ok()) {
      // Verify that the table is usable
      Iterator* it = table_cache->NewIterator(ReadOptions(), meta->number,
                                              meta->file_size);
      s = it->status();
      delete it;
    }
  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    // 如果 iterator 有问题或者文件大小为 0，则移除该文件
    env->RemoveFile(fname);
  }
  return s;
}

}  // namespace leveldb
