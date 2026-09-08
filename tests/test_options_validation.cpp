#include <catch2/catch_amalgamated.hpp>

#include <huxerui/codeeditor.h>

#include <stdexcept>

// The validator lives in the component implementation and is intentionally
// not part of the public header; tests bind it through its linkage name.
namespace huxerui::codeeditor {
void ValidateEditorOptions(const EditorOptions& options);
}

TEST_CASE("TestOptionsValidationAcceptsDefaultConfiguration") {
  huxerui::codeeditor::EditorOptions options;
  REQUIRE_NOTHROW(huxerui::codeeditor::ValidateEditorOptions(options));
}

TEST_CASE("TestOptionsValidationRejectsNonPositiveFontSizes") {
  huxerui::codeeditor::EditorOptions zero_size;
  zero_size.font_size = 0.0F;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(zero_size), std::invalid_argument);

  huxerui::codeeditor::EditorOptions negative_size;
  negative_size.font_size = -1.0F;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(negative_size), std::invalid_argument);
}

TEST_CASE("TestOptionsValidationRejectsNonPositiveTabSize") {
  huxerui::codeeditor::EditorOptions options;
  options.tab_size = 0;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(options), std::invalid_argument);
}

TEST_CASE("TestOptionsValidationRejectsNegativeLineSpacing") {
  huxerui::codeeditor::EditorOptions options;
  options.line_spacing_mult = -0.5F;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(options), std::invalid_argument);
}

TEST_CASE("TestOptionsValidationRejectsOutOfRangeDisplayModes") {
  huxerui::codeeditor::EditorOptions bad_wrap;
  bad_wrap.wrap_mode = 3;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(bad_wrap), std::invalid_argument);

  huxerui::codeeditor::EditorOptions negative_wrap;
  negative_wrap.wrap_mode = -1;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(negative_wrap), std::invalid_argument);

  huxerui::codeeditor::EditorOptions bad_scrollbar;
  bad_scrollbar.scrollbar_mode = 3;
  REQUIRE_THROWS_AS(huxerui::codeeditor::ValidateEditorOptions(bad_scrollbar), std::invalid_argument);
}

TEST_CASE("TestOptionsValidationKeepsBoundaryDisplayModesValid") {
  for (int wrap_mode = 0; wrap_mode <= 2; ++wrap_mode) {
    huxerui::codeeditor::EditorOptions options;
    options.wrap_mode = wrap_mode;
    options.scrollbar_mode = wrap_mode;
    REQUIRE_NOTHROW(huxerui::codeeditor::ValidateEditorOptions(options));
  }
}
