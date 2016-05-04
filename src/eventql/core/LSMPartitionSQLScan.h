/**
 * Copyright (c) 2015 - zScale Technology GmbH <legal@zscale.io>
 *   All Rights Reserved.
 *
 * Authors:
 *   Paul Asmuth <paul@zscale.io>
 *
 * This file is CONFIDENTIAL -- Distribution or duplication of this material or
 * the information contained herein is strictly forbidden unless prior written
 * permission is obtained.
 */
#pragma once
#include <eventql/util/stdtypes.h>
#include <eventql/util/protobuf/MessageSchema.h>
#include <csql/qtree/SequentialScanNode.h>
#include <csql/runtime/compiler.h>
#include <csql/runtime/defaultruntime.h>
#include <csql/runtime/TableExpression.h>
#include <csql/runtime/ValueExpression.h>
#include <cstable/CSTableReader.h>
#include <eventql/core/Table.h>
#include <eventql/core/PartitionReader.h>

using namespace stx;

namespace zbase {

class LSMPartitionSQLScan : public csql::TableExpression {
public:

  LSMPartitionSQLScan(
      csql::Transaction* ctx,
      RefPtr<Table> table,
      RefPtr<PartitionSnapshot> snap,
      RefPtr<csql::SequentialScanNode> stmt,
      csql::QueryBuilder* runtime);

  Vector<String> columnNames() const override;

  size_t numColumns() const override;

  void prepare(csql::ExecutionContext* context) override;

  void execute(
      csql::ExecutionContext* context,
      Function<bool (int argc, const csql::SValue* argv)> fn) override;

  Option<SHA1Hash> cacheKey() const override;
  void setCacheKey(const SHA1Hash& key);

protected:
  csql::Transaction* ctx_;
  RefPtr<Table> table_;
  RefPtr<PartitionSnapshot> snap_;
  RefPtr<csql::SequentialScanNode> stmt_;
  csql::QueryBuilder* runtime_;
  Vector<String> column_names_;
  Option<SHA1Hash> cache_key_;
  Set<SHA1Hash> id_set_;
};


} // namespace zbase