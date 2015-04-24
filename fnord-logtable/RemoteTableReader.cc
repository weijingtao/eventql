/**
 * This file is part of the "libfnord" project
 *   Copyright (c) 2015 Paul Asmuth
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <fnord-logtable/RemoteTableReader.h>
#include <fnord-msg/MessageDecoder.h>

namespace fnord {
namespace logtable {

RemoteTableReader::RemoteTableReader(
    const String& table_name,
    const msg::MessageSchema& schema,
    const URI& uri,
    http::HTTPConnectionPool* http) :
    name_(table_name),
    schema_(schema),
    uri_(uri),
    http_(http) {}

const String& RemoteTableReader::name() const {
  return name_;
}

const msg::MessageSchema& RemoteTableReader::schema() const {
  return schema_;
}

//RefPtr<TableSnapshot> RemoteTableReader::getSnapshot() {
//}

size_t RemoteTableReader::fetchRecords(
    const String& replica,
    uint64_t start_sequence,
    size_t limit,
    Function<bool (const msg::MessageObject& record)> fn) {
  auto path = StringUtil::format(
      "/fetch?table=$0&replica=$1&seq=$2&limit=$3",
      name_,
      replica,
      start_sequence,
      limit);

  URI uri(uri_.toString() + path);

  http::HTTPRequest req(http::HTTPMessage::M_GET, uri.pathAndQuery());
  req.addHeader("Host", uri.hostAndPort());
  req.addHeader("Content-Type", "application/fnord-msg");

  auto res = http_->executeRequest(req);
  res.wait();

  const auto& r = res.get();
  if (r.statusCode() != 200) {
    RAISEF(kRuntimeError, "received non-200 response: $0", r.body().toString());
  }

  const auto& buf = r.body();
  for (size_t offset = 0; offset < buf.size(); ) {
    msg::MessageObject msg;
    msg::MessageDecoder::decode(buf, schema_, &msg, &offset);

    if (!fn(msg)) {
      break;
    }
  }
}

} // namespace logtable
} // namespace fnord


