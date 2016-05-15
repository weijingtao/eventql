/**
 * Copyright (c) 2016 zScale Technology GmbH <legal@zscale.io>
 * Authors:
 *   - Paul Asmuth <paul@zscale.io>
 *   - Laura Schlimmer <laura@zscale.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#include <eventql/sql/qtree/RegexExpressionNode.h>
#include <eventql/sql/svalue.h>

using namespace util;

namespace csql {

RegexExpressionNode::RegexExpressionNode(
    RefPtr<ValueExpressionNode> subject,
    const String& pattern) :
    subject_(subject),
    pattern_(pattern) {}

Vector<RefPtr<ValueExpressionNode>> RegexExpressionNode::arguments() const {
  return Vector<RefPtr<ValueExpressionNode>>{ subject_ };
}

RefPtr<ValueExpressionNode> RegexExpressionNode::subject() const { 
  return subject_;
}

const String& RegexExpressionNode::pattern() const { 
  return pattern_;
}

RefPtr<QueryTreeNode> RegexExpressionNode::deepCopy() const {
  return new RegexExpressionNode(
      subject_->deepCopyAs<ValueExpressionNode>(),
      pattern_);
}

String RegexExpressionNode::toSQL() const {
  return StringUtil::format(
      "($0 REGEX '$1')",
      subject_->toSQL(),
      sql_escape(pattern_));
}

void RegexExpressionNode::encode(
    QueryTreeCoder* coder,
    const RegexExpressionNode& node,
    util::OutputStream* os) {
  coder->encode(node.subject_.get(), os);
  os->appendLenencString(node.pattern_);
}

RefPtr<QueryTreeNode> RegexExpressionNode::decode (
    QueryTreeCoder* coder,
    util::InputStream* is) {
  auto subject = coder->decode(is).asInstanceOf<ValueExpressionNode>();
  auto pattern = is->readLenencString();

  return new RegexExpressionNode(subject, pattern);
}


} // namespace csql
