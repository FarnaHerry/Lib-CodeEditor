#include <future>
#include <string>

#include <catch2/catch_amalgamated.hpp>
#include <huxerui/codeeditor.h>

TEST_CASE("PreparedDocumentPreservesUtf8AndLineEndings") {
  const std::string text = "first\r\n\xE4\xB8\xAD\xE6\x96\x87 \xF0\x9F\x98\x80\nlast\r";
  auto worker = std::async(std::launch::async, [text] {
    return huxerui::codeeditor::Document(text);
  });
  REQUIRE(worker.get().GetText() == text);
}

TEST_CASE("PreparedDocumentAcceptsEmptyAndLongSingleLine") {
  REQUIRE(huxerui::codeeditor::Document("").GetText().empty());
  const std::string text(1024 * 1024, 'x');
  const huxerui::codeeditor::Document document(text);
  REQUIRE(document.GetLineCount() == 1);
  REQUIRE(document.GetText() == text);
}
