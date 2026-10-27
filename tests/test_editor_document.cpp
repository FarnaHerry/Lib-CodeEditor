#include <future>
#include <string>

#include <catch2/catch_amalgamated.hpp>
#include <huxerui/codeeditor.h>

TEST_CASE("PreparedDocumentPreservesUtf8AndLineEndings") {
  const std::string text = "first\r\n\xE4\xB8\xAD\xE6\x96\x87 \xF0\x9F\x98\x80\nlast\r";
  auto worker = std::async(std::launch::async, [text] {
    return huxerui::codeeditor::PrepareEditorDocument(text);
  });
  const auto document = worker.get();
  REQUIRE(document);
  REQUIRE(document->getU8Text() == text);
}

TEST_CASE("PreparedDocumentAcceptsEmptyAndLongSingleLine") {
  REQUIRE(huxerui::codeeditor::PrepareEditorDocument("")->getU8Text().empty());
  const std::string text(1024 * 1024, 'x');
  const auto document = huxerui::codeeditor::PrepareEditorDocument(text);
  REQUIRE(document->getLineCount() == 1);
  REQUIRE(document->getU8Text() == text);
}
